/*
 * Epixel
 * Copyright (C) 2015 nerzhul, Loic Blot <loic.blot@unix-experience.fr>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/

#include <apr-1/apr_getopt.h>
#include <fstream>
#include <json/json.h>
#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/PatternLayout.hh>
#include <unistd.h>
#include "chatserver.h"
#include "globals.h"
#include "util.h"

apr_pool_t *g_apr_pool = NULL;
log4cpp::Category& logger = log4cpp::Category::getRoot();
std::map<std::string, std::string> startup_settings;
configReceptacle config;

static void cleanup_before_stop()
{
	apr_pool_destroy(g_apr_pool);
	apr_terminate();
}

static const apr_getopt_option_t _allowed_options[] = {
	/* long-option, short-option, has-arg flag, description */
	{ "help", 'h', false, "Show allowed options" },
	{ "log4cpp-props", 'L', true, "Set log4cpp properties file. If not, search log4cpp.properties in current dir,"
		" /usr/local/etc/emt-chatserver and /etc/emt-chatserver" },
	{ "bind", 'b', true, "Set listening address (default 0.0.0.0)" },
	{ "port", 'p', true, "Set listening port (default 30050)" },
	{ "config", 'c', true, "Set emt-chatserver configuration file. If not, search config.json in current dir,"
	  " /usr/local/etc/emt-chatserver and /etc/emt-chatserver" },
	{ "daemon", 'd', false, "Run process in background mode." },
	{ NULL, 0, 0, NULL },
};

static void print_help()
{
	logger.notice("%s", "Allowed options:");
	for (int i = 0; _allowed_options[i].name != NULL; i++) {
		std::ostringstream os1(std::ios::binary);
		os1 << "  --" << _allowed_options[i].name;
		if (_allowed_options[i].has_arg)
			os1 << " <value>";

		if (_allowed_options[i].description != NULL) {
			os1 << ": " << _allowed_options[i].description;
		}

		logger.noticeStream() << os1.str();
	}
}

static bool parse_command_line(int argc, char* argv[])
{
	apr_status_t rv;
	apr_getopt_t *opt;
	int optch;
	const char *optarg;
	apr_getopt_init(&opt, g_apr_pool, argc, argv);

	while ((rv = apr_getopt_long(opt, _allowed_options, &optch, &optarg)) == APR_SUCCESS) {
		switch (optch) {
			case 'h': startup_settings["help"] = "true"; break;
			case 'L': startup_settings["log4cpp_props"] = optarg; break;
			case 'b': startup_settings["bind"] = optarg; break;
			case 'p': startup_settings["port"] = optarg; break;
			case 'c': startup_settings["config"] = optarg; break;
			case 'd': startup_settings["daemonize"] = "true"; break;
			default: return false; break;
		}
	}

	if (rv != APR_EOF) {
		return false;
	}

	return true;
}

static void init_logging()
{
	std::string log4cpp_properties = "log4cpp.properties";
	if (startup_settings.find("log4cpp_props") != startup_settings.end()) {
		log4cpp_properties = startup_settings["log4cpp_props"];
	}
	else {
		static const std::vector<std::string> log4cpp_paths = {
			"./log4cpp.properties",
			"/usr/local/etc/emt-chatserver/log4cpp.properties",
			"/etc/emt-chatserver/log4cpp.properties"
		};
		for (const auto &p: log4cpp_paths) {
			if (filesystem::path_exists(p)) {
				log4cpp_properties = p;
				std::cout << "Selected " << p << " logging properties file." << std::endl;
				break;
			}
		}
	}

	try {
		log4cpp::PropertyConfigurator::configure(log4cpp_properties);
	}
	catch (log4cpp::ConfigureFailure &) {
		std::cout << "/!\\ Log4Cpp properties not found, logging enabled with default mode /!\\" << std::endl
				<< "To fix this warning please create log4cpp.properties and specify it with -L option" << std::endl
				<< "Enabling default logging in console mode with NOTICE logging level." << std::endl;

		logger.setPriority(log4cpp::Priority::NOTICE);
		log4cpp::Appender *console_appender = new log4cpp::OstreamAppender("console", &std::cout);
		log4cpp::PatternLayout* pl = new log4cpp::PatternLayout();
		pl->setConversionPattern("%d [%p] %m%n");
		console_appender->setLayout(pl);
		logger.addAppender(console_appender);
	}
}

static void init_config()
{
	std::string config_file = "config.json";
	if (startup_settings.find("config") != startup_settings.end()) {
		config_file = startup_settings["config"];
	}
	else {
		static const std::vector<std::string> log4cpp_paths = {
			"./config.json",
			"/usr/local/etc/emt-chatserver/config.json",
			"/etc/emt-chatserver/config.json"
		};
		for (const auto &p: log4cpp_paths) {
			if (filesystem::path_exists(p)) {
				config_file = p;
				logger.notice("Selected %s configuration file.", p.c_str());
				break;
			}
		}
	}

	if (filesystem::path_exists(config_file)) {
		Json::Value root;
		std::ifstream config_f(config_file, std::ifstream::binary);
		try {
			config_f >> root;
		}
		catch (std::exception &e) {
			logger.fatal("Invalid configuration file '%s'. Ensure this is a wellformed JSON file.",
					config_file.c_str());
			exit(1);
		}

		if (root.isMember("bind_address")) {
			if (!root["bind_address"].isString()) {
				logger.fatal("Config error: bind_address option should be a string");
				exit(1);
			}
			config.bind_addr = root["bind_address"].asString();
		}

		if (root.isMember("bind_port")) {
			if (!root["bind_port"].isInt()) {
				logger.fatal("Config error: bind_port option should be an integer");
				exit(1);
			}
			config.bind_port = root["bind_port"].asInt();
		}

		if (root.isMember("auth_token")) {
			if (!root["auth_token"].isString()) {
				logger.fatal("Config error: auth_token option should be an string");
				exit(1);
			}
			config.auth_token = root["auth_token"].asString();
		}
	}
	else {
		logger.notice("No configuration file found. Default settings loaded.");
	}

	// Read command line and override config file
	if (startup_settings.find("bind") != startup_settings.end()) {
		config.bind_addr = "0.0.0.0";
	}

	if (startup_settings.find("port") != startup_settings.end()) {
		config.bind_port = atoi(startup_settings["port"].c_str());
	}

	if (startup_settings.find("daemonize") != startup_settings.end()) {
		config.daemonize = true;
	}
}

int main(int argc, char *argv[])
{
	apr_initialize();
	apr_pool_create(&g_apr_pool, NULL);

	// Parse command line first
	startup_settings.clear();
	parse_command_line(argc, argv);

	// init log4cpp system
	init_logging();

	if (startup_settings.find("help") != startup_settings.end()) {
		print_help();
		exit(0);
	}

	init_config();

	pid_t pid = 0;
	if (config.daemonize) {
		pid = fork();
	}

	if (pid == 0) {
		epixel::ChatServer cs;
		cs.start();

		logger.notice("Bye !");
	}

	atexit(cleanup_before_stop);
	return 0;
}
