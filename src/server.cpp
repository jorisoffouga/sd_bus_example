#include <cerrno>
#include <vector>
#include <string>
#include <iostream>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <clocale>
#include <systemd/sd-bus.h>
#include <gpiod.hpp>

unsigned int temp = 46;

static const char bus_interface[] = "com.gateway.linux";
static const char bus_objet[] = "/com/gateway/linux";
static const char bus_name[] = "com.gateway.linux";

static int property(sd_bus *bus,
		const char *path,
		const char *interface,
		const char *property,
		sd_bus_message *reply,
		void *data,
		sd_bus_error *err)
{
	int ret;

	ret = sd_bus_message_append_basic(reply, 'x', &temp);
	if (ret < 0)
		return ret;

	return 1;
}

static int bus_signal_cb(sd_bus_message *reply, void *user_data, sd_bus_error *ret_error)
{
	int ret = 0;
	char * status;

	ret = sd_bus_message_read(reply, "s", &status);
	if (ret < 0) {
		std::cerr << "Failed to parse signal message: " << std::strerror(-ret);
		return -1;
	}

	std::cout << "Received : " << sd_bus_message_get_interface(reply) << " " <<
			sd_bus_message_get_path(reply) << " "<<  status << std::endl;

	return 0;
}

static int method_status(sd_bus_message *reply, void *userdata, sd_bus_error *ret_error)
{

	return sd_bus_reply_method_return(reply, "s", "HELLO B3");
}

static int method_reboot(sd_bus_message *reply, void *userdata, sd_bus_error *ret_error)
{
	int64_t x=0, y=0;
	char *c = nullptr;
	int ret = 0;

	/* Read the parameters */
	ret = sd_bus_message_read(reply, "sx", &c, &y);
	if (ret < 0) {
		std::cerr << "Failed to parse parameters: " << strerror(-ret);
		return ret;
	}

	temp = y;
	std::string res(c);

	std::cout << res.c_str() << " "<< y << std::endl;

	return sd_bus_reply_method_return(reply, "s", "OK");
}

static int method_set_gpio(sd_bus_message *reply, void *userdata, sd_bus_error *ret_error)
{
	int ret;
	int64_t offest;
	char *gpiochip = nullptr;
	bool state;

	ret = sd_bus_message_read(reply, "snb", &gpiochip, &offest, &state);
	if (ret < 0){
		std::cerr << "Failed to parse parameters: " << strerror(-ret) << std::endl;
		return ret;
	}

	gpiod::chip chip(gpiochip);
	auto line = chip.get_line(offest);
	line.request({"sd-server", gpiod::line_request::DIRECTION_OUTPUT,0}, state);
	line.release();

	return sd_bus_reply_method_return(reply, "s", "OK");
}

static const sd_bus_vtable b3_vtable[] = {
		SD_BUS_VTABLE_START(0),
		SD_BUS_METHOD("GetStatus", "", "s", method_status, SD_BUS_VTABLE_UNPRIVILEGED),
		SD_BUS_METHOD("Reboot", "sx", "s", method_reboot, SD_BUS_VTABLE_UNPRIVILEGED),
		SD_BUS_PROPERTY("GetTemp","x", property, 0, SD_BUS_VTABLE_PROPERTY_CONST),
		SD_BUS_METHOD("SetGpio", "nb", "s", method_set_gpio, SD_BUS_VTABLE_UNPRIVILEGED),
		SD_BUS_VTABLE_END

};

int main(int argc, char *argv[]) {
	sd_bus_error error = SD_BUS_ERROR_NULL;
	sd_bus_slot *slot = nullptr;
	sd_bus_message *reply = nullptr;
	sd_bus *bus = nullptr;
	const char *path;
	int ret;

	/* Connect to the system bus */
#ifdef USE_HOST
	ret = sd_bus_open_user(&bus);
#else
	ret = sd_bus_open_system(&bus);
#endif

	if (ret < 0) {
		std::cerr << "Failed to connect to system bus: " << std::strerror(-ret) << std::endl;
		goto finish;
	}

	/* Install the object */
	ret = sd_bus_add_object_vtable(bus,
			&slot,
			bus_objet,  	   /* object path */
			bus_interface,   /* interface name */
			b3_vtable,
			NULL);

	if (ret < 0) {
		std::cerr << "Failed to issue method call: " << std::strerror(-ret) << std::endl;
		goto finish;
	}

	ret = sd_bus_request_name(bus, bus_name, 0);
	if (ret < 0) {
		std::cerr << "Failed to acquire service name: " << std::strerror(-ret) << std::endl;
		goto finish;
	}

	ret = sd_bus_add_match(bus, &slot,
			"path='/Test',"
			"type='signal',"
			"interface='com.dbus.test',"
			"member='status'",
			bus_signal_cb, nullptr);

	if (ret < 0) {
		std::cerr << "Failed: sd_bus_add_match: " << std::strerror(-ret) << std::endl;
		goto finish;
	}

	for (;;) {
		/* Process requests */
		ret = sd_bus_process(bus, nullptr);

		if (ret < 0) {
			std::cerr << "Failed to process bus: " << std::strerror(-ret) << std::endl;
			goto finish;
		}

		if (ret > 0) /* we processed a request, try to process another one, right-away */
			continue;

		/* Wait for the next request to process */
		ret = sd_bus_wait(bus, static_cast<uint64_t>(-1));

		if (ret < 0) {
			std::cerr << "Failed to wait on bus: " <<  std::strerror(-ret) << std::endl;
			goto finish;
		}
	}

finish:
	sd_bus_error_free(&error);
	sd_bus_message_unref(reply);
	sd_bus_unref(bus);

	return ret < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
