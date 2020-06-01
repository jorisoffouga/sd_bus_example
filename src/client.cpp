#include <vector>
#include <cstring>
#include <string>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <systemd/sd-bus.h>

static const char bus_interface[] = "com.gateway.linux";
static const char bus_objet[] = "/com/gateway/linux";
static const char bus_name[] = "com.gateway.linux";

int main(int argc, char *argv[]) {
	sd_bus_error error = SD_BUS_ERROR_NULL;
	sd_bus_message *reply = nullptr;
	sd_bus *bus = nullptr;
	char *status = nullptr;
	int temp;
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

	ret = sd_bus_get_property(bus,
			bus_interface,
			bus_objet,
			bus_interface,
			"GetTemp",
			&error,
			&reply, "x");
	if (ret < 0) {
		std::cerr << "Failed to get property call: " << error.message << std::endl;
		goto finish;
	}

	/* Parse the response message */
	ret = sd_bus_message_read(reply, "x", &temp);
	if (ret < 0) {
		std::cerr <<  "Failed to parse response message: " << std::strerror(-ret) << std::endl;
		goto finish;
	}

	std::cout << "Temperature: " << temp << std::endl;

	ret = sd_bus_call_method(bus,
			bus_interface, /* service to contact */
			bus_objet, /* object path */
			bus_name, /* interface name */
			"Reboot", /* method name */
			&error,/* object to return error in */
			&reply,
			"sx",
			"Reboot",
			46
	);
	/* input signature */
	/* first argument */
	/* second argument */
	if (ret < 0) {
		std::cerr << "Failed to issue method call: " << error.message << std::endl;
		goto finish;
	}

	/* Parse the response message */
	ret = sd_bus_message_read(reply, "s", &status);
	if (ret < 0) {
		std::cerr <<  "Failed to parse response message: " << std::strerror(-ret) << std::endl;
		goto finish;
	}

	std::cout << "Reboot: " << status << std::endl;


	ret = sd_bus_call_method(bus,
			bus_interface, /* service to contact */
			bus_objet, /* object path */
			bus_name, /* interface name */
			"GetStatus", /* method name */
			&error,/* object to return error in */
			&reply,
			nullptr
	);
	/* input signature */
	/* first argument */
	/* second argument */
	if (ret < 0) {
		std::cerr << "Failed to issue method call: " << error.message << std::endl;
		goto finish;
	}

	/* Parse the response message */
	ret = sd_bus_message_read(reply, "s", &status);
	if (ret < 0) {
		std::cerr <<  "Failed to parse response message: " << std::strerror(-ret) << std::endl;
		goto finish;
	}

	std::cout << "GetStatus: " << status << std::endl;

	ret = sd_bus_emit_signal(bus,"/Test", "com.dbus.test", "status", "s", "Hello");

	if (ret < 0){
		std::cerr << "Failed to send signal: " << std::strerror(-ret) << std::endl;
		goto finish;
	}

finish:
	sd_bus_error_free(&error);
	sd_bus_message_unref(reply);
	sd_bus_unref(bus);

	return ret < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
