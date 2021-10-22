/**
 * collectd - src/redfish_test.c
 *
 * Copyright(c) 2018 Intel Corporation. All rights reserved.
 * Copyright(c) 2021 Atos. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Original authors:
 *   Martin Kennelly <martin.kennelly@intel.com>
 *   Marcin Mozejko <marcinx.mozejko@intel.com>
 *   Adrian Boczkowski <adrianx.boczkowski@intel.com>
 *
 * Refactoring and enhancement author:
 *      Mathieu Stoffel <mathieu.stoffel@atos.net>
 **/

#define plugin_dispatch_values redfish_test_plugin_dispatch_values_mock

#include "redfish.c"
#include "testing.h"

#define VALUE_CACHE_SIZE (1)

static value_list_t last_dispatched_value_list;
static value_t last_dispatched_values[VALUE_CACHE_SIZE];

/*******/

/* To be allocated to build a configuration file: */
static oconfig_item_t* config_file                 = NULL;

/* Pointers to specific sub-parts of interest of the built configuration
 * file: */
#define CONFIG_FILE_SERVICES (1)
#define CONFIG_FILE_QUERIES  (5)
#define CONFIG_FILE_SUBPARTS (CONFIG_FILE_SERVICES + CONFIG_FILE_QUERIES)
static oconfig_item_t* cf_service            = NULL;
static oconfig_item_t* cf_query_thermal      = NULL;
static oconfig_item_t* cf_query_voltages     = NULL;
static oconfig_item_t* cf_query_temperatures = NULL;
static oconfig_item_t* cf_query_ps1_voltage  = NULL;
static oconfig_item_t* cf_query_storage      = NULL;

/*******/

/* Builds a configuration file in memory so as to be able to properly test each
 * individual internal sub-function of the plugin.
 * The built configuration file is to be cleaned up by "destroy_config_file": */
static uint64_t build_config_file(void)
{
    /* Utilitary pointers to be used later on in order to make the code
     * associated with the building of the configuration file clearer: */
    oconfig_item_t* cf_resource  = NULL;
    oconfig_item_t* cf_property  = NULL;
    oconfig_item_t* cf_attribute = NULL;

    /**************************************************************************
     * Root:
     **************************************************************************/
    /* Allocating the root of the configuration file: */
    config_file = calloc(1, sizeof(*config_file));
    if (config_file == NULL) return EXIT_FAILURE;
    /***/
    config_file->key = "redfish";
    config_file->parent = NULL;

    /* Allocating the queries and services belonging to the configuration file,
     * which are the children of its root: */
    config_file->children_num = CONFIG_FILE_SUBPARTS;
    /***/
    config_file->children =
        calloc(config_file->children_num, sizeof(*(config_file->children)));
    /***/
    if (config_file->children == NULL) return EXIT_FAILURE;

    /* Assigning the pointers specific to sub-parts of the configuration
     * files: */
    cf_service            = &(config_file->children[0]);
    cf_query_thermal      = &(config_file->children[1]);
    cf_query_voltages     = &(config_file->children[2]);
    cf_query_temperatures = &(config_file->children[3]);
    cf_query_ps1_voltage  = &(config_file->children[4]);
    cf_query_storage      = &(config_file->children[5]);

    /**************************************************************************
     * Service subpart:
     **************************************************************************/
    /* Allocating the root of the Service subpart of the configuration file: */
    cf_service->key = "Service";
    cf_service->parent = config_file;
    /***/
    cf_service->values_num = 1;
    cf_service->values = calloc(
        cf_service->values_num, sizeof(*(cf_service->values))
    );
    /***/
    if (cf_service->values == NULL) return EXIT_FAILURE;
    /***/
    cf_service->values[0].type = OCONFIG_TYPE_STRING;
    cf_service->values[0].value.string = "mock1U";

    /* Allocating the children of the Service subpart: */
    cf_service->children_num = 4;
    cf_service->children = calloc(
        cf_service->children_num, sizeof(*(cf_service->children))
    );
    /***/
    if (cf_service->children == NULL) return EXIT_FAILURE;

    /* Filling in the information related to the host: */
    cf_service->children[0].key = "Host";
    cf_service->children[0].parent = cf_service;
    /***/
    cf_service->children[0].values_num = 1;
    cf_service->children[0].values = calloc(
        cf_service->children[0].values_num, 
        sizeof(*(cf_service->children[0].values))
    );
    /***/
    if (cf_service->children[0].values == NULL) return EXIT_FAILURE;
    /***/
    cf_service->children[0].values[0].type = OCONFIG_TYPE_STRING;
    cf_service->children[0].values[0].value.string = "localhost:10000";

    /* Filling in the information related to the user: */
    cf_service->children[1].key = "User";
    cf_service->children[1].parent = cf_service;
    /***/
    cf_service->children[1].values_num = 1;
    cf_service->children[1].values = calloc(
        cf_service->children[1].values_num, 
        sizeof(*(cf_service->children[1].values))
    );
    /***/
    if (cf_service->children[1].values == NULL) return EXIT_FAILURE;
    /***/
    cf_service->children[1].values[0].type = OCONFIG_TYPE_STRING;
    cf_service->children[1].values[0].value.string = "";

    /* Filling in the information related to the password: */
    cf_service->children[2].key = "Passwd";
    cf_service->children[2].parent = cf_service;
    /***/
    cf_service->children[2].values_num = 1;
    cf_service->children[2].values = calloc(
        cf_service->children[2].values_num, 
        sizeof(*(cf_service->children[2].values))
    );
    /***/
    if (cf_service->children[2].values == NULL) return EXIT_FAILURE;
    /***/
    cf_service->children[2].values[0].type = OCONFIG_TYPE_STRING;
    cf_service->children[2].values[0].value.string = "";

    /* Filling in the information related to the queries: */
    cf_service->children[3].key = "Queries";
    cf_service->children[3].parent = cf_service;
    /***/
    cf_service->children[3].values_num = CONFIG_FILE_QUERIES;
    cf_service->children[3].values = calloc(
        cf_service->children[3].values_num, 
        sizeof(*(cf_service->children[3].values))
    );
    /***/
    if (cf_service->children[3].values == NULL) return EXIT_FAILURE;
    /***/
    cf_service->children[3].values[0].type = OCONFIG_TYPE_STRING;
    cf_service->children[3].values[0].value.string = "thermal";
    /***/
    cf_service->children[3].values[1].type = OCONFIG_TYPE_STRING;
    cf_service->children[3].values[1].value.string = "voltages";
    /***/
    cf_service->children[3].values[2].type = OCONFIG_TYPE_STRING;
    cf_service->children[3].values[2].value.string = "temperatures";
    /***/
    cf_service->children[3].values[3].type = OCONFIG_TYPE_STRING;
    cf_service->children[3].values[3].value.string = "ps1_voltage";
    /***/
    cf_service->children[3].values[4].type = OCONFIG_TYPE_STRING;
    cf_service->children[3].values[4].value.string = "storage";

    /**************************************************************************
     * Query "fans" subpart:
     **************************************************************************/
    /* Allocating the root of the Query "thermal" subpart of the configuration
     * file: */
    cf_query_thermal->key = "Query";
    cf_query_thermal->parent = config_file;
    /***/
    cf_query_thermal->values_num = 1;
    cf_query_thermal->values = calloc(
        cf_query_thermal->values_num, sizeof(*(cf_query_thermal->values))
    );
    /***/
    if (cf_query_thermal->values == NULL) return EXIT_FAILURE;
    /***/
    cf_query_thermal->values[0].type = OCONFIG_TYPE_STRING;
    cf_query_thermal->values[0].value.string = "thermal";

    /* Allocating the children of the Query "fans" subpart: */
    cf_query_thermal->children_num = 3;
    cf_query_thermal->children = calloc(
        cf_query_thermal->children_num, sizeof(*(cf_query_thermal->children))
    );
    /***/
    if (cf_query_thermal->children == NULL) return EXIT_FAILURE;

    /* Filling in the information related to the endpoint: */
    cf_query_thermal->children[0].key = "Endpoint";
    cf_query_thermal->children[0].parent = cf_query_thermal;
    /***/
    cf_query_thermal->children[0].values_num = 1;
    cf_query_thermal->children[0].values = calloc(
        cf_query_thermal->children[0].values_num, 
        sizeof(*(cf_query_thermal->children[0].values))
    );
    /***/
    if (cf_query_thermal->children[0].values == NULL) return EXIT_FAILURE;
    /***/
    cf_query_thermal->children[0].values[0].type = OCONFIG_TYPE_STRING;
    cf_query_thermal->children[0].values[0].value.string =
        "/Chassis[0]/Thermal";

    /* Filling in the information related to the resource: */
    cf_resource = &(cf_query_thermal->children[1]);
    cf_resource->key = "Resource";
    cf_resource->parent = cf_query_thermal;
    /***/
    cf_resource->values_num = 1;
    cf_resource->values = calloc(
        cf_resource->values_num, sizeof(*(cf_resource->values))
    );
    /***/
    if (cf_resource->values == NULL) return EXIT_FAILURE;
    /***/
    cf_resource->values[0].type = OCONFIG_TYPE_STRING;
    cf_resource->values[0].value.string = "Fans";

    /* Allocating the property associated with the first resource: */
    cf_resource->children_num = 1;
    cf_resource->children = calloc(
        cf_resource->children_num, sizeof(*(cf_resource->children))
    );
    /***/
    if (cf_resource->children == NULL) return EXIT_FAILURE;
    /***/
    cf_property = &(cf_resource->children[0]);

    /* Filling in the information related to the property: */
    cf_property->key = "Property";
    cf_property->parent = cf_resource;
    /***/
    cf_property->values_num = 1;
    cf_property->values = calloc(
        cf_property->values_num, sizeof(*(cf_property->values))
    );
    /***/
    if (cf_property->values == NULL) return EXIT_FAILURE;
    /***/
    cf_property->values[0].type = OCONFIG_TYPE_STRING;
    cf_property->values[0].value.string = "Reading";

    /* Allocating the fields (i.e. children) of the property: */
    cf_property->children_num = 4;
    cf_property->children = calloc(
        cf_property->children_num, sizeof(*(cf_property->children))
    );
    /***/
    if (cf_property->children == NULL) return EXIT_FAILURE;

    /* Filling in the information related to the PluginInstance field of the
     * property: */
    cf_property->children[0].parent = cf_property;
    cf_property->children[0].key = "PluginInstance";
    /***/
    cf_property->children[0].values_num = 1;
    cf_property->children[0].values = calloc(
        cf_property->children[0].values_num,
        sizeof(*(cf_property->children[0].values))
    );
    /***/
    if (cf_property->children[0].values == NULL) return EXIT_FAILURE;
    /***/
    cf_property->children[0].values[0].type = OCONFIG_TYPE_STRING;
    cf_property->children[0].values[0].value.string = "Fans";

    /* Filling in the information related to the Type field of the property: */
    cf_property->children[1].parent = cf_property;
    cf_property->children[1].key = "Type";
    /***/
    cf_property->children[1].values_num = 1;
    cf_property->children[1].values = calloc(
        cf_property->children[1].values_num,
        sizeof(*(cf_property->children[1].values))
    );
    /***/
    if (cf_property->children[1].values == NULL) return EXIT_FAILURE;
    /***/
    cf_property->children[1].values[0].type = OCONFIG_TYPE_STRING;
    cf_property->children[1].values[0].value.string = "fanspeed";

    /* Filling in the information related to the TypeInstanceAttr field of the
     * property: */
    cf_property->children[2].parent = cf_property;
    cf_property->children[2].key = "TypeInstanceAttr";
    /***/
    cf_property->children[2].values_num = 1;
    cf_property->children[2].values = calloc(
        cf_property->children[2].values_num,
        sizeof(*(cf_property->children[2].values))
    );
    /***/
    if (cf_property->children[2].values == NULL) return EXIT_FAILURE;
    /***/
    cf_property->children[2].values[0].type = OCONFIG_TYPE_STRING;
    cf_property->children[2].values[0].value.string = "Name";

    /* Filling in the information related to the SelectIDs field of the
     * property: */
    cf_property->children[3].parent = cf_property;
    cf_property->children[3].key = "SelectIDs";
    /***/
    cf_property->children[3].values_num = 1;
    cf_property->children[3].values = calloc(
        cf_property->children[1].values_num,
        sizeof(*(cf_property->children[3].values))
    );
    /***/
    if (cf_property->children[3].values == NULL) return EXIT_FAILURE;
    /***/
    cf_property->children[3].values[0].type = OCONFIG_TYPE_NUMBER;
    cf_property->children[3].values[0].value.number = 1;

    /* Filling in the information related to the second resource: */
    cf_resource = &(cf_query_thermal->children[2]);
    cf_resource->key = "Resource";
    cf_resource->parent = cf_query_thermal;
    /***/
    cf_resource->values_num = 1;
    cf_resource->values = calloc(
        cf_resource->values_num, sizeof(*(cf_resource->values))
    );
    /***/
    if (cf_resource->values == NULL) return EXIT_FAILURE;
    /***/
    cf_resource->values[0].type = OCONFIG_TYPE_STRING;
    cf_resource->values[0].value.string = "Temperatures";

    /* Allocating the property associated with the resource: */
    cf_resource->children_num = 1;
    cf_resource->children = calloc(
        cf_resource->children_num, sizeof(*(cf_resource->children))
    );
    /***/
    if (cf_resource->children == NULL) return EXIT_FAILURE;
    /***/
    cf_property = &(cf_resource->children[0]);

    /* Filling in the information related to the property: */
    cf_property->key = "Property";
    cf_property->parent = cf_resource;
    /***/
    cf_property->values_num = 1;
    cf_property->values = calloc(
        cf_property->values_num, sizeof(*(cf_property->values))
    );
    /***/
    if (cf_property->values == NULL) return EXIT_FAILURE;
    /***/
    cf_property->values[0].type = OCONFIG_TYPE_STRING;
    cf_property->values[0].value.string = "ReadingCelsius";

    /* Allocating the fields (i.e. children) of the property: */
    cf_property->children_num = 3;
    cf_property->children = calloc(
        cf_property->children_num, sizeof(*(cf_property->children))
    );
    /***/
    if (cf_property->children == NULL) return EXIT_FAILURE;

    /* Filling in the information related to the PluginInstance field of the
     * property: */
    cf_property->children[0].parent = cf_property;
    cf_property->children[0].key = "PluginInstance";
    /***/
    cf_property->children[0].values_num = 1;
    cf_property->children[0].values = calloc(
        cf_property->children[0].values_num,
        sizeof(*(cf_property->children[0].values))
    );
    /***/
    if (cf_property->children[0].values == NULL) return EXIT_FAILURE;
    /***/
    cf_property->children[0].values[0].type = OCONFIG_TYPE_STRING;
    cf_property->children[0].values[0].value.string = "Temperatures";

    /* Filling in the information related to the Type field of the property: */
    cf_property->children[1].parent = cf_property;
    cf_property->children[1].key = "Type";
    /***/
    cf_property->children[1].values_num = 1;
    cf_property->children[1].values = calloc(
        cf_property->children[1].values_num,
        sizeof(*(cf_property->children[1].values))
    );
    /***/
    if (cf_property->children[1].values == NULL) return EXIT_FAILURE;
    /***/
    cf_property->children[1].values[0].type = OCONFIG_TYPE_STRING;
    cf_property->children[1].values[0].value.string = "temperature";

    /* Filling in the information related to the SelectAttrValue field of the
     * property: */
    cf_property->children[2].parent = cf_property;
    cf_property->children[2].key = "SelectAttrValue";
    /***/
    cf_property->children[2].values_num = 2;
    cf_property->children[2].values = calloc(
        cf_property->children[2].values_num,
        sizeof(*(cf_property->children[2].values))
    );
    /***/
    if (cf_property->children[2].values == NULL) return EXIT_FAILURE;
    /***/
    cf_property->children[2].values[0].type = OCONFIG_TYPE_STRING;
    cf_property->children[2].values[0].value.string = "PhysicalContext";
    /***/
    cf_property->children[2].values[1].type = OCONFIG_TYPE_STRING;
    cf_property->children[2].values[1].value.string = "Intake";

    /**************************************************************************
     * Query "voltages" subpart:
     **************************************************************************/
    /* Allocating the root of the Query "voltages" subpart of the
     * configuration file: */
    cf_query_voltages->key = "Query";
    cf_query_voltages->parent = config_file;
    /***/
    cf_query_voltages->values_num = 1;
    cf_query_voltages->values = calloc(
        cf_query_voltages->values_num,
        sizeof(*(cf_query_voltages->values))
    );
    /***/
    if (cf_query_voltages->values == NULL) return EXIT_FAILURE;
    /***/
    cf_query_voltages->values[0].type = OCONFIG_TYPE_STRING;
    cf_query_voltages->values[0].value.string = "voltages";

    /* Allocating the children of the Query "voltages" subpart: */
    cf_query_voltages->children_num = 2;
    cf_query_voltages->children = calloc(
        cf_query_voltages->children_num,
        sizeof(*(cf_query_voltages->children))
    );
    /***/
    if (cf_query_voltages->children == NULL) return EXIT_FAILURE;

    /* Filling in the information related to the endpoint: */
    cf_query_voltages->children[0].key = "Endpoint";
    cf_query_voltages->children[0].parent = cf_query_voltages;
    /***/
    cf_query_voltages->children[0].values_num = 1;
    cf_query_voltages->children[0].values = calloc(
        cf_query_voltages->children[0].values_num, 
        sizeof(*(cf_query_voltages->children[0].values))
    );
    /***/
    if (cf_query_voltages->children[0].values == NULL) return EXIT_FAILURE;
    /***/
    cf_query_voltages->children[0].values[0].type = OCONFIG_TYPE_STRING;
    cf_query_voltages->children[0].values[0].value.string = "/Chassis[0]/Power";

    /* Filling in the information related to the resource: */
    cf_resource = &(cf_query_voltages->children[1]);
    cf_resource->key = "Resource";
    cf_resource->parent = cf_query_voltages;
    /***/
    cf_resource->values_num = 1;
    cf_resource->values = calloc(
        cf_resource->values_num, sizeof(*(cf_resource->values))
    );
    /***/
    if (cf_resource->values == NULL) return EXIT_FAILURE;
    /***/
    cf_resource->values[0].type = OCONFIG_TYPE_STRING;
    cf_resource->values[0].value.string = "Voltages";

    /* Allocating the property associated with the resource: */
    cf_resource->children_num = 1;
    cf_resource->children = calloc(
        cf_resource->children_num, sizeof(*(cf_resource->children))
    );
    /***/
    if (cf_resource->children == NULL) return EXIT_FAILURE;
    /***/
    cf_property = &(cf_resource->children[0]);

    /* Filling in the information related to the property: */
    cf_property->key = "Property";
    cf_property->parent = cf_resource;
    /***/
    cf_property->values_num = 1;
    cf_property->values = calloc(
        cf_property->values_num, sizeof(*(cf_property->values))
    );
    /***/
    if (cf_property->values == NULL) return EXIT_FAILURE;
    /***/
    cf_property->values[0].type = OCONFIG_TYPE_STRING;
    cf_property->values[0].value.string = "ReadingVolts";

    /* Allocating the fields (i.e. children) of the property: */
    cf_property->children_num = 4;
    cf_property->children = calloc(
        cf_property->children_num, sizeof(*(cf_property->children))
    );
    /***/
    if (cf_property->children == NULL) return EXIT_FAILURE;

    /* Filling in the information related to the PluginInstance field of the
     * property: */
    cf_property->children[0].parent = cf_property;
    cf_property->children[0].key = "PluginInstance";
    /***/
    cf_property->children[0].values_num = 1;
    cf_property->children[0].values = calloc(
        cf_property->children[0].values_num,
        sizeof(*(cf_property->children[0].values))
    );
    /***/
    if (cf_property->children[0].values == NULL) return EXIT_FAILURE;
    /***/
    cf_property->children[0].values[0].type = OCONFIG_TYPE_STRING;
    cf_property->children[0].values[0].value.string = "Voltages";

    /* Filling in the information related to the Type field of the property: */
    cf_property->children[1].parent = cf_property;
    cf_property->children[1].key = "Type";
    /***/
    cf_property->children[1].values_num = 1;
    cf_property->children[1].values = calloc(
        cf_property->children[1].values_num,
        sizeof(*(cf_property->children[1].values))
    );
    /***/
    if (cf_property->children[1].values == NULL) return EXIT_FAILURE;
    /***/
    cf_property->children[1].values[0].type = OCONFIG_TYPE_STRING;
    cf_property->children[1].values[0].value.string = "voltages";

    /* Filling in the information related to the TypeInstance field of the
     * property: */
    cf_property->children[2].parent = cf_property;
    cf_property->children[2].key = "TypeInstance";
    /***/
    cf_property->children[2].values_num = 1;
    cf_property->children[2].values = calloc(
        cf_property->children[2].values_num,
        sizeof(*(cf_property->children[2].values))
    );
    /***/
    if (cf_property->children[2].values == NULL) return EXIT_FAILURE;
    /***/
    cf_property->children[2].values[0].type = OCONFIG_TYPE_STRING;
    cf_property->children[2].values[0].value.string = "VRM";

    /* Filling in the information related to the TypeInstance field of the
     * property: */
    cf_property->children[3].parent = cf_property;
    cf_property->children[3].key = "TypeInstancePrefixID";
    /***/
    cf_property->children[3].values_num = 1;
    cf_property->children[3].values = calloc(
        cf_property->children[3].values_num,
        sizeof(*(cf_property->children[3].values))
    );
    /***/
    if (cf_property->children[3].values == NULL) return EXIT_FAILURE;
    /***/
    cf_property->children[3].values[0].type = OCONFIG_TYPE_BOOLEAN;
    cf_property->children[3].values[0].value.boolean = true;

    /**************************************************************************
     * Query "temperatures" subpart:
     **************************************************************************/
    /* Allocating the root of the Query "temperatures" subpart of the
     * configuration file: */
    cf_query_temperatures->key = "Query";
    cf_query_temperatures->parent = config_file;
    /***/
    cf_query_temperatures->values_num = 1;
    cf_query_temperatures->values = calloc(
        cf_query_temperatures->values_num,
        sizeof(*(cf_query_temperatures->values))
    );
    /***/
    if (cf_query_temperatures->values == NULL) return EXIT_FAILURE;
    /***/
    cf_query_temperatures->values[0].type = OCONFIG_TYPE_STRING;
    cf_query_temperatures->values[0].value.string = "temperatures";

    /* Allocating the children of the Query "temperatures" subpart: */
    cf_query_temperatures->children_num = 2;
    cf_query_temperatures->children = calloc(
        cf_query_temperatures->children_num,
        sizeof(*(cf_query_temperatures->children))
    );
    /***/
    if (cf_query_temperatures->children == NULL) return EXIT_FAILURE;

    /* Filling in the information related to the endpoint: */
    cf_query_temperatures->children[0].key = "Endpoint";
    cf_query_temperatures->children[0].parent = cf_query_temperatures;
    /***/
    cf_query_temperatures->children[0].values_num = 1;
    cf_query_temperatures->children[0].values = calloc(
        cf_query_temperatures->children[0].values_num, 
        sizeof(*(cf_query_temperatures->children[0].values))
    );
    /***/
    if (cf_query_temperatures->children[0].values == NULL) return EXIT_FAILURE;
    /***/
    cf_query_temperatures->children[0].values[0].type = OCONFIG_TYPE_STRING;
    cf_query_temperatures->children[0].values[0].value.string =
        "/Chassis[0]/ThermalSubsystem/ThermalMetrics";

    /* Filling in the information related to the resource: */
    cf_resource = &(cf_query_temperatures->children[1]);
    cf_resource->key = "Resource";
    cf_resource->parent = cf_query_temperatures;
    /***/
    cf_resource->values_num = 1;
    cf_resource->values = calloc(
        cf_resource->values_num, sizeof(*(cf_resource->values))
    );
    /***/
    if (cf_resource->values == NULL) return EXIT_FAILURE;
    /***/
    cf_resource->values[0].type = OCONFIG_TYPE_STRING;
    cf_resource->values[0].value.string = "TemperatureReadingsCelsius";

    /* Allocating the property associated with the resource: */
    cf_resource->children_num = 1;
    cf_resource->children = calloc(
        cf_resource->children_num, sizeof(*(cf_resource->children))
    );
    /***/
    if (cf_resource->children == NULL) return EXIT_FAILURE;
    /***/
    cf_property = &(cf_resource->children[0]);

    /* Filling in the information related to the property: */
    cf_property->key = "Property";
    cf_property->parent = cf_resource;
    /***/
    cf_property->values_num = 1;
    cf_property->values = calloc(
        cf_property->values_num, sizeof(*(cf_property->values))
    );
    /***/
    if (cf_property->values == NULL) return EXIT_FAILURE;
    /***/
    cf_property->values[0].type = OCONFIG_TYPE_STRING;
    cf_property->values[0].value.string = "Reading";

    /* Allocating the fields (i.e. children) of the property: */
    cf_property->children_num = 3;
    cf_property->children = calloc(
        cf_property->children_num, sizeof(*(cf_property->children))
    );
    /***/
    if (cf_property->children == NULL) return EXIT_FAILURE;

    /* Filling in the information related to the PluginInstance field of the
     * property: */
    cf_property->children[0].parent = cf_property;
    cf_property->children[0].key = "PluginInstance";
    /***/
    cf_property->children[0].values_num = 1;
    cf_property->children[0].values = calloc(
        cf_property->children[0].values_num,
        sizeof(*(cf_property->children[0].values))
    );
    /***/
    if (cf_property->children[0].values == NULL) return EXIT_FAILURE;
    /***/
    cf_property->children[0].values[0].type = OCONFIG_TYPE_STRING;
    cf_property->children[0].values[0].value.string = "Temperatures";

    /* Filling in the information related to the Type field of the property: */
    cf_property->children[1].parent = cf_property;
    cf_property->children[1].key = "Type";
    /***/
    cf_property->children[1].values_num = 1;
    cf_property->children[1].values = calloc(
        cf_property->children[1].values_num,
        sizeof(*(cf_property->children[1].values))
    );
    /***/
    if (cf_property->children[1].values == NULL) return EXIT_FAILURE;
    /***/
    cf_property->children[1].values[0].type = OCONFIG_TYPE_STRING;
    cf_property->children[1].values[0].value.string = "temperature";

    /* Filling in the information related to the TypeInstanceAttr field of the
     * property: */
    cf_property->children[2].parent = cf_property;
    cf_property->children[2].key = "TypeInstanceAttr";
    /***/
    cf_property->children[2].values_num = 1;
    cf_property->children[2].values = calloc(
        cf_property->children[2].values_num,
        sizeof(*(cf_property->children[2].values))
    );
    /***/
    if (cf_property->children[2].values == NULL) return EXIT_FAILURE;
    /***/
    cf_property->children[2].values[0].type = OCONFIG_TYPE_STRING;
    cf_property->children[2].values[0].value.string = "DeviceName";

    /**************************************************************************
     * Query "ps1_voltage" subpart:
     **************************************************************************/
    /* Allocating the root of the Query "ps1_voltage" subpart of the
     * configuration file: */
    cf_query_ps1_voltage->key = "Query";
    cf_query_ps1_voltage->parent = config_file;
    /***/
    cf_query_ps1_voltage->values_num = 1;
    cf_query_ps1_voltage->values = calloc(
        cf_query_ps1_voltage->values_num,
        sizeof(*(cf_query_ps1_voltage->values))
    );
    /***/
    if (cf_query_ps1_voltage->values == NULL) return EXIT_FAILURE;
    /***/
    cf_query_ps1_voltage->values[0].type = OCONFIG_TYPE_STRING;
    cf_query_ps1_voltage->values[0].value.string = "ps1_voltage";

    /* Allocating the children of the Query "ps1_voltage" subpart: */
    cf_query_ps1_voltage->children_num = 2;
    cf_query_ps1_voltage->children = calloc(
        cf_query_ps1_voltage->children_num,
        sizeof(*(cf_query_ps1_voltage->children))
    );
    /***/
    if (cf_query_ps1_voltage->children == NULL) return EXIT_FAILURE;

    /* Filling in the information related to the endpoint: */
    cf_query_ps1_voltage->children[0].key = "Endpoint";
    cf_query_ps1_voltage->children[0].parent = cf_query_ps1_voltage;
    /***/
    cf_query_ps1_voltage->children[0].values_num = 1;
    cf_query_ps1_voltage->children[0].values = calloc(
        cf_query_ps1_voltage->children[0].values_num, 
        sizeof(*(cf_query_ps1_voltage->children[0].values))
    );
    /***/
    if (cf_query_ps1_voltage->children[0].values == NULL) return EXIT_FAILURE;
    /***/
    cf_query_ps1_voltage->children[0].values[0].type = OCONFIG_TYPE_STRING;
    cf_query_ps1_voltage->children[0].values[0].value.string =
        "/Chassis[0]/Sensors[15]";

    /* Filling in the information related to the attribute: */
    cf_attribute = &(cf_query_ps1_voltage->children[1]);
    cf_attribute->key = "Attribute";
    cf_attribute->parent = cf_query_ps1_voltage;
    /***/
    cf_attribute->values_num = 1;
    cf_attribute->values = calloc(
        cf_attribute->values_num, sizeof(*(cf_attribute->values))
    );
    /***/
    if (cf_attribute->values == NULL) return EXIT_FAILURE;
    /***/
    cf_attribute->values[0].type = OCONFIG_TYPE_STRING;
    cf_attribute->values[0].value.string = "Reading";

    /* Allocating the fields (i.e. children) of the attribute: */
    cf_attribute->children_num = 3;
    cf_attribute->children = calloc(
        cf_attribute->children_num, sizeof(*(cf_attribute->children))
    );
    /***/
    if (cf_attribute->children == NULL) return EXIT_FAILURE;

    /* Filling in the information related to the PluginInstance field of the
     * attribute: */
    cf_attribute->children[0].parent = cf_attribute;
    cf_attribute->children[0].key = "PluginInstance";
    /***/
    cf_attribute->children[0].values_num = 1;
    cf_attribute->children[0].values = calloc(
        cf_attribute->children[0].values_num,
        sizeof(*(cf_attribute->children[0].values))
    );
    /***/
    if (cf_attribute->children[0].values == NULL) return EXIT_FAILURE;
    /***/
    cf_attribute->children[0].values[0].type = OCONFIG_TYPE_STRING;
    cf_attribute->children[0].values[0].value.string = "Voltages";

    /* Filling in the information related to the Type field of the attribute: */
    cf_attribute->children[1].parent = cf_attribute;
    cf_attribute->children[1].key = "Type";
    /***/
    cf_attribute->children[1].values_num = 1;
    cf_attribute->children[1].values = calloc(
        cf_attribute->children[1].values_num,
        sizeof(*(cf_attribute->children[1].values))
    );
    /***/
    if (cf_attribute->children[1].values == NULL) return EXIT_FAILURE;
    /***/
    cf_attribute->children[1].values[0].type = OCONFIG_TYPE_STRING;
    cf_attribute->children[1].values[0].value.string = "voltage";

    /* Filling in the information related to the TypeInstance field of the
     * attribute: */
    cf_attribute->children[2].parent = cf_attribute;
    cf_attribute->children[2].key = "TypeInstance";
    /***/
    cf_attribute->children[2].values_num = 1;
    cf_attribute->children[2].values = calloc(
        cf_attribute->children[2].values_num,
        sizeof(*(cf_attribute->children[2].values))
    );
    /***/
    if (cf_attribute->children[2].values == NULL) return EXIT_FAILURE;
    /***/
    cf_attribute->children[2].values[0].type = OCONFIG_TYPE_STRING;
    cf_attribute->children[2].values[0].value.string = "PS1 Voltage";

    /**************************************************************************
     * Query "storage" subpart:
     **************************************************************************/
    /* Allocating the root of the Query "storage" subpart of the configuration
     * file: */
    cf_query_storage->key = "Query";
    cf_query_storage->parent = config_file;
    /***/
    cf_query_storage->values_num = 1;
    cf_query_storage->values = calloc(
        cf_query_storage->values_num, sizeof(*(cf_query_storage->values))
    );
    /***/
    if (cf_query_storage->values == NULL) return EXIT_FAILURE;
    /***/
    cf_query_storage->values[0].type = OCONFIG_TYPE_STRING;
    cf_query_storage->values[0].value.string = "storage";

    /* Allocating the children of the Query "storage" subpart: */
    cf_query_storage->children_num = 2;
    cf_query_storage->children = calloc(
        cf_query_storage->children_num, sizeof(*(cf_query_storage->children))
    );
    /***/
    if (cf_query_storage->children == NULL) return EXIT_FAILURE;

    /* Filling in the information related to the endpoint: */
    cf_query_storage->children[0].key = "Endpoint";
    cf_query_storage->children[0].parent =
        cf_query_storage;
    /***/
    cf_query_storage->children[0].values_num = 1;
    cf_query_storage->children[0].values = calloc(
        cf_query_storage->children[0].values_num, 
        sizeof(*(cf_query_storage->children[0].values))
    );
    /***/
    if (cf_query_storage->children[0].values == NULL) return EXIT_FAILURE;
    /***/
    cf_query_storage->children[0].values[0].type = OCONFIG_TYPE_STRING;
    cf_query_storage->children[0].values[0].value.string =
        "/Chassis[0]/SimpleStorage[0]";

    /* Filling in the information related to the resource: */
    cf_resource = &(cf_query_storage->children[1]);
    cf_resource->key = "Resource";
    cf_resource->parent = cf_query_storage;
    /***/
    cf_resource->values_num = 1;
    cf_resource->values = calloc(
        cf_resource->values_num, sizeof(*(cf_resource->values))
    );
    /***/
    if (cf_resource->values == NULL) return EXIT_FAILURE;
    /***/
    cf_resource->values[0].type = OCONFIG_TYPE_STRING;
    cf_resource->values[0].value.string = "Devices";

    /* Allocating the property associated with the resource: */
    cf_resource->children_num = 1;
    cf_resource->children = calloc(
        cf_resource->children_num, sizeof(*(cf_resource->children))
    );
    /***/
    if (cf_resource->children == NULL) return EXIT_FAILURE;
    /***/
    cf_property = &(cf_resource->children[0]);

    /* Filling in the information related to the property: */
    cf_property->key = "Property";
    cf_property->parent = cf_resource;
    /***/
    cf_property->values_num = 1;
    cf_property->values = calloc(
        cf_property->values_num, sizeof(*(cf_property->values))
    );
    /***/
    if (cf_property->values == NULL) return EXIT_FAILURE;
    /***/
    cf_property->values[0].type = OCONFIG_TYPE_STRING;
    cf_property->values[0].value.string = "CapacityBytes";

    /* Allocating the fields (i.e. children) of the property: */
    cf_property->children_num = 3;
    cf_property->children = calloc(
        cf_property->children_num, sizeof(*(cf_property->children))
    );
    /***/
    if (cf_property->children == NULL) return EXIT_FAILURE;

    /* Filling in the information related to the PluginInstance field of the
     * property: */
    cf_property->children[0].parent = cf_property;
    cf_property->children[0].key = "PluginInstance";
    /***/
    cf_property->children[0].values_num = 1;
    cf_property->children[0].values = calloc(
        cf_property->children[0].values_num,
        sizeof(*(cf_property->children[0].values))
    );
    /***/
    if (cf_property->children[0].values == NULL) return EXIT_FAILURE;
    /***/
    cf_property->children[0].values[0].type = OCONFIG_TYPE_STRING;
    cf_property->children[0].values[0].value.string = "Storage";

    /* Filling in the information related to the Type field of the property: */
    cf_property->children[1].parent = cf_property;
    cf_property->children[1].key = "Type";
    /***/
    cf_property->children[1].values_num = 1;
    cf_property->children[1].values = calloc(
        cf_property->children[1].values_num,
        sizeof(*(cf_property->children[1].values))
    );
    /***/
    if (cf_property->children[1].values == NULL) return EXIT_FAILURE;
    /***/
    cf_property->children[1].values[0].type = OCONFIG_TYPE_STRING;
    cf_property->children[1].values[0].value.string = "capacity";

    /* Filling in the information related to the SelectAttrs field of the
     * property: */
    cf_property->children[2].parent = cf_property;
    cf_property->children[2].key = "SelectAttrs";
    /***/
    cf_property->children[2].values_num = 2;
    cf_property->children[2].values = calloc(
        cf_property->children[2].values_num,
        sizeof(*(cf_property->children[2].values))
    );
    /***/
    if (cf_property->children[2].values == NULL) return EXIT_FAILURE;
    /***/
    cf_property->children[2].values[0].type = OCONFIG_TYPE_STRING;
    cf_property->children[2].values[0].value.string = "Model";
    /***/
    cf_property->children[2].values[1].type = OCONFIG_TYPE_STRING;
    cf_property->children[2].values[1].value.string = "Name";

    return EXIT_SUCCESS;
}

/*******/

static void destroy_config_file_query_with_resource(
    const oconfig_item_t* const query
)
{
    /* Utilitary pointers to be used later on in order to make the code
     * associated with the deallocation of the query clearer: */
    oconfig_item_t* cf_resource  = NULL;
    oconfig_item_t* cf_property  = NULL;

    /* Checking that the specified query should be deallocated: */
    if (query != NULL)
    {
        /* Deallocating the values associated with the query: */
        if (query->values != NULL) free(query->values);

        /* Deallocating the children of the query, after individually
         * deallocating their content: */
        if (query->children != NULL)
        {
            /* Deallocating the endpoint: */
            if (query->children[0].values != NULL)
            {
                free(query->children[0].values);
            }

            /* Extracting the resource part of the query: */
            cf_resource = &(query->children[1]);

            /* Deallocating the resource's values: */
            if (cf_resource->values != NULL) free(cf_resource->values);

            /* Deallocating the resource's children: */
            if (cf_resource->children != NULL)
            {
                /* Extracting the property part of the resource: */
                cf_property = &(cf_resource->children[0]);

                /* Deallocating the property's values: */
                if (cf_property->values != NULL) free(cf_property->values);

                /* Deallocating the property's children/fields: */
                if (cf_property->children != NULL)
                {
                    for (int i = 0 ; i < cf_property->children_num ; i++)
                    {
                        if (cf_property->children[i].values != NULL)
                        {
                            free(cf_property->children[i].values);
                        }
                    }

                    free(cf_property->children);
                }

                free(cf_resource->children);
            }

            free(query->children);
        }
    }
}

/*******/

static void destroy_config_file_query_with_attribute(
    const oconfig_item_t* const query
)
{
    /* Utilitary pointers to be used later on in order to make the code
     * associated with the deallocation of the query clearer: */
    oconfig_item_t* cf_attribute  = NULL;

    /* Checking that the specified query should be deallocated: */
    if (query != NULL)
    {
        /* Deallocating the values associated with the query: */
        if (query->values != NULL) free(query->values);

        /* Deallocating the children of the query, after individually
         * deallocating their content: */
        if (query->children != NULL)
        {
            /* Deallocating the endpoint: */
            if (query->children[0].values != NULL)
            {
                free(query->children[0].values);
            }

            /* Extracting the attribute part of the resource: */
            cf_attribute = &(query->children[1]);

            /* Deallocating the attribute's values: */
            if (cf_attribute->values != NULL) free(cf_attribute->values);

            /* Deallocating the attribute's children/fields: */
            if (cf_attribute->children != NULL)
            {
                for (int i = 0 ; i < cf_attribute->children_num ; i++)
                {
                    if (cf_attribute->children[i].values != NULL)
                    {
                        free(cf_attribute->children[i].values);
                    }
                }

                free(cf_attribute->children);
            }

            free(query->children);
        }
    }
}

/*******/

/* Destroys the in-memory configuration file built by "build_config_file".
 * To do so, this function deallocates, if required, each sub-part (queries and
 * services) of the configuration file, before deallocating its root: */
static void destroy_config_file(void)
{
    /**************************************************************************
     * Service:
     **************************************************************************/
    if (cf_service != NULL)
    {
        /* Deallocating the values associated with the Service: */
        if (cf_service->values != NULL) free(cf_service->values);

        /* Deallocating the children of the service, after individually
         * deallocating their content: */
        if (cf_service->children != NULL)
        {
            for (int i = 0 ; i < cf_service->children_num ; i++)
            {
                if (cf_service->children[i].values != NULL)
                {
                    free(cf_service->children[i].values);
                }
            }

            free(cf_service->children);
        }
    }

    /**************************************************************************
     * Queries:
     **************************************************************************/
    destroy_config_file_query_with_resource(cf_query_thermal);
    destroy_config_file_query_with_resource(cf_query_voltages);
    destroy_config_file_query_with_resource(cf_query_temperatures);
    destroy_config_file_query_with_attribute(cf_query_ps1_voltage);
    destroy_config_file_query_with_resource(cf_query_storage);

    /**************************************************************************
     * Root:
     **************************************************************************/
    if (config_file != NULL)
    {
        /* Deallocating the values associated with the root: */
        if (config_file->values != NULL) free(config_file->values);

        /* Deallocating the children associated with the root, that is to say
         * the service and queries, which content was already deallocated: */
        if (config_file->children != NULL) free(config_file->children);

        /* Deallocating the root itself: */
        free(config_file);
    }

    /**************************************************************************
     * Global reset:
     **************************************************************************/
    config_file           = NULL;
    cf_service            = NULL;
    cf_query_thermal      = NULL;
    cf_query_voltages     = NULL;
    cf_query_temperatures = NULL;
    cf_query_ps1_voltage  = NULL;
    cf_query_storage      = NULL;
}

/*******/

/* Mocking the dispatch of sampled values: */
int redfish_test_plugin_dispatch_values_mock(value_list_t const * vl)
{
    last_dispatched_value_list = *vl;
    size_t len = MIN(vl->values_len, VALUE_CACHE_SIZE);

    for (size_t i = 0; i < len; ++i) last_dispatched_values[i] = vl->values[i];

    last_dispatched_value_list.values = last_dispatched_values;

    return 0;
}

/*******/

/* Getting the list of the last dispatched sampled values: */
static value_list_t* redfish_test_get_last_dispatched_value_list()
{
    return (&last_dispatched_value_list);
}

/*******/

/* Conversion of parsed data types to collectd's data types: */
DEF_TEST(convert_val)
{
    redfish_value_t val = {.string = "1"};
    redfish_value_type_t src_type = VAL_TYPE_STR;
    int dst_type = DS_TYPE_GAUGE;
    value_t vl = {0};
    int ret = redfish_convert_val(&val, src_type, &vl, dst_type);

    EXPECT_EQ_INT(0, ret);
    OK(vl.gauge == 1.0);

    val.integer = 1;
    src_type = VAL_TYPE_INT;
    dst_type = DS_TYPE_GAUGE;
    ret = redfish_convert_val(&val, src_type, &vl, dst_type);
    EXPECT_EQ_INT(0, ret);
    OK(vl.gauge == 1.0);

    val.real = 1.0;
    src_type = VAL_TYPE_REAL;
    dst_type = DS_TYPE_GAUGE;
    ret = redfish_convert_val(&val, src_type, &vl, dst_type);
    EXPECT_EQ_INT(0, ret);
    OK(vl.gauge == 1.0);

    val.string = "-1";
    src_type = VAL_TYPE_STR;
    dst_type = DS_TYPE_DERIVE;
    ret = redfish_convert_val(&val, src_type, &vl, dst_type);
    EXPECT_EQ_INT(0, ret);
    OK(vl.derive == -1);

    val.integer = -1;
    src_type = VAL_TYPE_INT;
    dst_type = DS_TYPE_DERIVE;
    ret = redfish_convert_val(&val, src_type, &vl, dst_type);
    EXPECT_EQ_INT(0, ret);
    OK(vl.derive == -1);

    val.real = -1.0;
    src_type = VAL_TYPE_REAL;
    dst_type = DS_TYPE_DERIVE;
    ret = redfish_convert_val(&val, src_type, &vl, dst_type);
    EXPECT_EQ_INT(0, ret);
    OK(vl.derive == -1);

    val.string = "1";
    src_type = VAL_TYPE_STR;
    dst_type = DS_TYPE_COUNTER;
    ret = redfish_convert_val(&val, src_type, &vl, dst_type);
    EXPECT_EQ_INT(0, ret);
    OK(vl.counter == 1);

    val.integer = 1;
    src_type = VAL_TYPE_INT;
    dst_type = DS_TYPE_COUNTER;
    ret = redfish_convert_val(&val, src_type, &vl, dst_type);
    EXPECT_EQ_INT(0, ret);
    OK(vl.counter == 1);

    val.real = 1.0;
    src_type = VAL_TYPE_REAL;
    dst_type = DS_TYPE_COUNTER;
    ret = redfish_convert_val(&val, src_type, &vl, dst_type);
    EXPECT_EQ_INT(0, ret);
    OK(vl.counter == 1);

    val.string = "1";
    src_type = VAL_TYPE_STR;
    dst_type = DS_TYPE_ABSOLUTE;
    ret = redfish_convert_val(&val, src_type, &vl, dst_type);
    EXPECT_EQ_INT(0, ret);
    OK(vl.absolute == 1);

    val.integer = 1;
    src_type = VAL_TYPE_INT;
    dst_type = DS_TYPE_ABSOLUTE;
    ret = redfish_convert_val(&val, src_type, &vl, dst_type);
    EXPECT_EQ_INT(0, ret);
    OK(vl.absolute == 1);

    val.real = 1.0;
    src_type = VAL_TYPE_REAL;
    dst_type = DS_TYPE_ABSOLUTE;
    ret = redfish_convert_val(&val, src_type, &vl, dst_type);
    EXPECT_EQ_INT(0, ret);
    OK(vl.absolute == 1);

    return 0;
}

/*******/

/* Testing the memory allocation for the context structure.
 * Creation of services list & queries AVL tree: */
DEF_TEST(redfish_preconfig)
{
    int ret = redfish_preconfig();

    EXPECT_EQ_INT(0, ret);
    CHECK_NOT_NULL(ctx.queries);
    CHECK_NOT_NULL(ctx.services);

    llist_destroy(ctx.services);
    c_avl_destroy(ctx.queries);

    return 0;
}

/*******/

/* Reading the names of the queries from the configuration files: */
//DEF_TEST(read_queries)
//{
//    oconfig_item_t* ci = calloc(1, sizeof(*ci));
//
//    CHECK_NOT_NULL(ci);
//    ci->values = calloc(6, sizeof(*ci->values));
//    CHECK_NOT_NULL(ci->values);
//
//    ci->values_num = 6;
//    ci->values[0].value.string = "temperatures";
//    ci->values[0].type = OCONFIG_TYPE_STRING;
//    ci->values[1].value.string = "fans";
//    ci->values[1].type = OCONFIG_TYPE_STRING;
//    ci->values[2].value.string = "voltages";
//    ci->values[2].type = OCONFIG_TYPE_STRING;
//    ci->values[3].value.string = "ps1_voltage";
//    ci->values[3].type = OCONFIG_TYPE_STRING;
//    ci->values[4].value.string = "intake_temperature";
//    ci->values[4].type = OCONFIG_TYPE_STRING;
//    ci->values[5].value.string = "storage";
//    ci->values[5].type = OCONFIG_TYPE_STRING;
//
//    char** queries;
//    int ret = redfish_read_queries(ci, &queries);
//
//    EXPECT_EQ_INT(0, ret);
//    EXPECT_EQ_STR("temperatures", queries[0]);
//    EXPECT_EQ_STR("fans", queries[1]);
//    EXPECT_EQ_STR("voltages", queries[2]);
//    EXPECT_EQ_STR("ps1_voltage", queries[3]);
//    EXPECT_EQ_STR("intake_temperature", queries[4]);
//    EXPECT_EQ_STR("storage", queries[5]);
//
//    sfree(ci->values);
//    sfree(ci);
//
//    for (int j = 0 ; j < 6 ; j++) sfree(queries[j]);
//    /***/
//    sfree(queries);
//
//    return 0;
//}
//
///*******/
//
///* Testing correct input of properties from the configuration file: */
//DEF_TEST(config_property)
//{
//    redfish_resource_t* resource = calloc(1, sizeof(*resource));
//    CHECK_NOT_NULL(resource);
//    resource->name = "test property";
//    resource->properties = llist_create();
//    CHECK_NOT_NULL(resource->properties);
//
//    oconfig_item_t* ci = calloc(1, sizeof(*ci));
//    CHECK_NOT_NULL(ci);
//
//    ci->values_num = 1;
//    ci->values = calloc(1, sizeof(*ci->values));
//    ci->values[0].type = OCONFIG_TYPE_STRING;
//    ci->values[0].value.string = "ReadingCelsius";
//
//    ci->children_num = 3;
//    ci->children = calloc(1, sizeof(*ci->children) * ci->children_num);
//
//    ci->children[0].key = "PluginInstance";
//    ci->children[0].parent = ci;
//    ci->children[0].values = calloc(1, sizeof(*ci->children[0].values));
//    CHECK_NOT_NULL(ci->children[0].values);
//    ci->children[0].values_num = 1;
//    (ci->children[0].values[0]).value.string = "Temperatures";
//    (ci->children[0].values[0]).type = OCONFIG_TYPE_STRING;
//
//    ci->children[1].key = "Type";
//    ci->children[1].parent = ci;
//    ci->children[1].values = calloc(1, sizeof(*ci->children[1].values));
//    CHECK_NOT_NULL(ci->children[1].values);
//    ci->children[1].values_num = 1;
//    (ci->children[1].values[0]).value.string = "temperature";
//    (ci->children[1].values[0]).type = OCONFIG_TYPE_STRING;
//
//    ci->children[2].key = "TypeInstance";
//    ci->children[2].parent = ci;
//    ci->children[2].values = calloc(1, sizeof(*ci->children[2].values));
//    CHECK_NOT_NULL(ci->children[2].values);
//    ci->children[2].values_num = 1;
//    (ci->children[2].values[0]).value.string = "PH-TypeInstance";
//    (ci->children[2].values[0]).type = OCONFIG_TYPE_STRING;
//
//    ci->children[3].key = "TypeInstanceAttr";
//    ci->children[3].parent = ci;
//    ci->children[3].values = calloc(1, sizeof(*ci->children[3].values));
//    CHECK_NOT_NULL(ci->children[3].values);
//    ci->children[3].values_num = 1;
//    (ci->children[3].values[0]).value.string = "PH-TypeInstanceAttr";
//    (ci->children[3].values[0]).type = OCONFIG_TYPE_STRING;
//
//    ci->children[4].key = "TypeInstancePrefixID";
//    ci->children[4].parent = ci;
//    ci->children[4].values = calloc(1, sizeof(*ci->children[4].values));
//    CHECK_NOT_NULL(ci->children[4].values);
//    ci->children[4].values_num = 1;
//    (ci->children[4].values[0]).value.boolean = true;
//    (ci->children[4].values[0]).type = OCONFIG_TYPE_BOOLEAN;
//
//    ci->children[5].key = "SelectAttrs";
//    ci->children[5].parent = ci;
//    ci->children[5].values = calloc(2, sizeof(*ci->children[5].values));
//    CHECK_NOT_NULL(ci->children[5].values);
//    ci->children[5].values_num = 2;
//    (ci->children[5].values[0]).value.string = "PhysicalContext";
//    (ci->children[5].values[0]).type = OCONFIG_TYPE_STRING;
//    (ci->children[5].values[1]).value.string = "Capacity";
//    (ci->children[5].values[1]).type = OCONFIG_TYPE_STRING;
//
//    ci->children[6].key = "SelectAttrValue";
//    ci->children[6].parent = ci;
//    ci->children[6].values = calloc(2, sizeof(*ci->children[6].values));
//    CHECK_NOT_NULL(ci->children[6].values);
//    ci->children[6].values_num = 2;
//    (ci->children[6].values[0]).value.string = "PH-SelectAttr";
//    (ci->children[6].values[0]).type = OCONFIG_TYPE_STRING;
//    (ci->children[6].values[1]).value.string = "PH-SelectValue";
//    (ci->children[6].values[1]).type = OCONFIG_TYPE_STRING;
//
//    int ret = redfish_config_property(resource, ci);
//
//    EXPECT_EQ_INT(0, ret);
//    EXPECT_EQ_INT(1, llist_size(resource->properties));
//
//    for (uint64_t i = 0 ; i < 7 ; i++) sfree(ci->children[i].values);
//    /***/
//    sfree(ci->children);
//    sfree(ci->values);
//    sfree(ci);
//
//    for
//    (
//        llentry_t* llprop = llist_head(resource->properties) ;
//        llprop != NULL ;
//        llprop = llprop->next
//    )
//    {
//        redfish_property_t* property = (redfish_property_t*)llprop->value;
//
//        sfree(property->name);
//        sfree(property->plugin_inst);
//        sfree(property->type);
//        sfree(property->type_inst);
//        sfree(property->type_inst_attr);
//        strarray_free(property->select_attrs, property->nb_select_attrs);
//        sfree(property->select_ids);
//
//        llentry_t* current = llist_head(property->select_attrvalues);
//        /***/
//        while (current != NULL)
//        {
//            sfree(current->key);
//            sfree(current->value);
//
//            current = current->next;
//        }
//        /***/
//        llist_destroy(property->select_attrvalues);
//
//        sfree(property);
//    }
//    /***/
//    llist_destroy(resource->properties);
//    free(resource);
//
//    return 0;
//}
//
///*******/
//
//DEF_TEST(config_resource)
//{
//    oconfig_item_t* ci = calloc(1, sizeof(*ci));
//    assert(ci != NULL);
//
//    ci->values_num = 1;
//    ci->values = calloc(1, sizeof(*ci->values));
//    assert(ci->values != NULL);
//    ci->values[0].value.string = "Temperatures";
//
//    ci->children_num = 1;
//    ci->children = calloc(1, sizeof(*ci->children));
//    assert(ci->children != NULL);
//
//    ci->children[0].children_num = 1;
//    ci->children[0].parent = ci;
//    ci->children[0].key = "Property";
//    ci->children[0].values_num = 1;
//    ci->children[0].values = calloc(1, sizeof(*ci->children[0].values));
//    ci->children[0].values->value.string = "ReadingRPM";
//    assert(ci->children[0].values != NULL);
//
//    oconfig_item_t* ci_prop = calloc(1, sizeof(*ci_prop));
//    assert(ci_prop != NULL);
//
//    ci->children[0].children = ci_prop;
//    ci->children_num = 1;
//
//    ci_prop->key = "PluginInstance";
//    ci_prop->parent = ci;
//    ci_prop->values_num = 1;
//    ci_prop->values = calloc(1, sizeof(*ci_prop->values));
//    assert(ci_prop->values != NULL);
//    ci_prop->values[0].type = OCONFIG_TYPE_STRING;
//    ci_prop->values[0].value.string = "chassis-1";
//
//    redfish_query_t* query = calloc(1, sizeof(*query));
//    query->endpoint = "/redfish/v1/Chassis/Chassis-1/Thermal";
//    query->name = "fans";
//    query->resources = llist_create();
//
//    int ret = redfish_config_resource(query, ci);
//    EXPECT_EQ_INT(0, ret);
//    EXPECT_EQ_INT(1, llist_size(query->resources));
//
//    sfree(ci_prop->values);
//    sfree(ci_prop);
//
//    sfree(ci->values);
//    sfree(ci->children[0].values);
//    sfree(ci->children);
//    sfree(ci);
//
//    for
//    (
//        llentry_t* llres = llist_head(query->resources) ;
//        llres != NULL;
//        llres = llres->next
//    )
//    {
//        redfish_resource_t* resource = (redfish_resource_t*)llres->value;
//        /***/
//        for
//        (
//            llentry_t* llprop = llist_head(resource->properties) ;
//            llprop != NULL;
//            llprop = llprop->next
//        )
//        {
//            redfish_property_t* property = (redfish_property_t*)llprop->value;
//        
//            sfree(property->name);
//            sfree(property->plugin_inst);
//            sfree(property->type);
//            sfree(property->type_inst);
//            free(property);
//        }
//        /***/
//        llist_destroy(resource->properties);
//        free(resource->name);
//        free(resource);
//    }
//    /***/
//    llist_destroy(query->resources);
//    sfree(query);
//
//    return 0;
//}
//
///*******/
//
//DEF_TEST(config_query)
//{
//    oconfig_item_t* qci = calloc(1, sizeof(*qci));
//    assert(qci != NULL);
//
//    qci->key = "Query";
//    qci->values_num = 1;
//    qci->values = calloc(1, sizeof(*qci->values));
//    assert(qci->values != NULL);
//    qci->values->type = OCONFIG_TYPE_STRING;
//    qci->values->value.string = "fans";
//
//    qci->children_num = 2;
//    qci->children = calloc(1, sizeof(*qci->children) * qci->children_num);
//    assert(qci->children != NULL);
//
//    qci->children[0].key = "Endpoint";
//    qci->children[0].values = calloc(1, sizeof(*qci->children[0].values));
//    assert(qci->children[0].values != NULL);
//    qci->children[0].values->type = OCONFIG_TYPE_STRING;
//    qci->children[0].values_num = 1;
//    qci->children[0].values->value.string =
//        "/redfish/v1/Chassis/Chassis-1/Thermal";
//
//    qci->children[1].key = "Resource";
//    qci->children[1].values_num = 1;
//    qci->children[1].values = calloc(1, sizeof(*qci->children[1].values));
//    assert(qci->children[1].values != NULL);
//    qci->children[1].values->type = OCONFIG_TYPE_STRING;
//    qci->children[1].values->value.string = "Temperature";
//    qci->children[1].children_num = 1;
//
//    oconfig_item_t* ci = calloc(1, sizeof(*ci));
//    assert(ci != NULL);
//
//    qci->children[1].children = ci;
//
//    ci->key = "Property";
//    ci->values_num = 1;
//    ci->values = calloc(1, sizeof(*ci->values));
//    assert(ci->values != NULL);
//    ci->values->type = OCONFIG_TYPE_STRING;
//    ci->values->value.string = "ReadingRPM";
//
//    oconfig_item_t* ci_prop = calloc(1, sizeof(*ci_prop));
//    assert(ci_prop != NULL);
//
//    ci->children = ci_prop;
//    ci->children_num = 1;
//
//    ci_prop->key = "PluginInstance";
//    ci_prop->parent = ci;
//    ci_prop->values_num = 1;
//    ci_prop->values = calloc(1, sizeof(*ci_prop->values));
//    assert(ci_prop->values != NULL);
//    ci_prop->values[0].type = OCONFIG_TYPE_STRING;
//    ci_prop->values[0].value.string = "chassis-1";
//
//    c_avl_tree_t* queries =
//        c_avl_create((int (*)(const void*, const void*))strcmp);
//
//    int ret = redfish_config_query(qci, queries);
//    EXPECT_EQ_INT(0, ret);
//
//    sfree(ci_prop->values);
//    sfree(ci_prop);
//
//    sfree(ci->values);
//    sfree(ci);
//    sfree(qci->children[0].values);
//    sfree(qci->children[1].values);
//    sfree(qci->children);
//    sfree(qci->values);
//    sfree(qci);
//
//    redfish_query_t* query;
//    char* key;
//    c_avl_iterator_t* query_iter = c_avl_get_iterator(queries);
//
//    while
//    (
//        c_avl_iterator_next(
//            query_iter, 
//            (void**)(&key),
//            (void**)(&query)
//        ) == 0
//    )
//    {
//        for
//        (
//            llentry_t* llres = llist_head(query->resources) ;
//            llres != NULL;
//            llres = llres->next
//        )
//        {
//            redfish_resource_t* resource = (redfish_resource_t*)llres->value;
//            /***/
//            for
//            (
//                llentry_t* llprop = llist_head(resource->properties) ;
//                llprop != NULL ;
//                llprop = llprop->next
//            )
//            {
//                redfish_property_t* property =
//                    (redfish_property_t*)llprop->value;
//
//                sfree(property->name);
//                sfree(property->plugin_inst);
//                sfree(property->type);
//                sfree(property->type_inst);
//                sfree(property);
//            }
//            /***/
//            llist_destroy(resource->properties);
//            sfree(resource->name);
//            sfree(resource);
//        }
//        /***/
//        llist_destroy(query->resources);
//        sfree(query->name);
//        sfree(query->endpoint);
//    }
//    /***/
//    sfree(query_iter);
//    c_avl_destroy(queries);
//
//    return 0;
//}
//
///*******/
//
//DEF_TEST(config_service)
//{
//    oconfig_item_t *ci = calloc(1, sizeof(*ci));
//    assert(ci != NULL);
//
//    ci->key = "Service";
//    ci->values_num = 1;
//    ci->values = calloc(1, sizeof(*ci->values));
//    ci->values->type = OCONFIG_TYPE_STRING;
//    ci->values->value.string = "Server 5";
//    ci->children_num = 4;
//    ci->children = calloc(1, sizeof(*ci->children) * ci->children_num);
//    ci->children[0].key = "Host";
//    ci->children[0].values_num = 1;
//    ci->children[0].values = calloc(1, sizeof(*ci->children[0].values));
//    ci->children[0].values->type = OCONFIG_TYPE_STRING;
//    ci->children[0].values->value.string = "127.0.0.1:5000";
//    ci->children[1].key = "User";
//    ci->children[1].values_num = 1;
//    ci->children[1].values = calloc(1, sizeof(*ci->children[1].values));
//    ci->children[1].values->type = OCONFIG_TYPE_STRING;
//    ci->children[1].values->value.string = "user";
//    ci->children[2].key = "Passwd";
//    ci->children[2].values_num = 1;
//    ci->children[2].values = calloc(1, sizeof(*ci->children[2].values));
//    ci->children[2].values->type = OCONFIG_TYPE_STRING;
//    ci->children[2].values->value.string = "passwd";
//    ci->children[3].key = "Queries";
//    ci->children[3].values_num = 1;
//    ci->children[3].values = calloc(1, sizeof(*ci->children[3].values));
//    ci->children[3].values->type = OCONFIG_TYPE_STRING;
//    ci->children[3].values->value.string = "fans";
//
//    ctx.services = llist_create();
//
//    int ret = redfish_config_service(ci);
//
//    EXPECT_EQ_INT(0, ret);
//
//    for
//    (
//        llentry_t* llserv = llist_head(ctx.services) ;
//        llserv != NULL ;
//        llserv = llserv->next
//    )
//    {
//        redfish_service_t* serv = (redfish_service_t*)(llserv->value);
//        sfree(serv->name);
//        sfree(serv->host);
//        sfree(serv->user);
//        sfree(serv->passwd);
//        /***/
//        for (uint64_t i = 0 ; i < serv->queries_num ; i++)
//        {
//            sfree(serv->queries[i]);
//        }
//        /***/
//        sfree(serv->queries);
//        sfree(serv);
//    }
//    /***/
//    llist_destroy(ctx.services);
//
//    sfree(ci->children[3].values);
//    sfree(ci->children[2].values);
//    sfree(ci->children[1].values);
//    sfree(ci->children[0].values);
//    sfree(ci->children);
//    sfree(ci->values);
//    sfree(ci);
//
//    return 0;
//}
//
///*******/
//
///*TODO: handle multiple cases -> resource => array
// *                            -> resource => object
// *                            -> attribute */
//DEF_TEST(process_payload_property)
//{
//    redfish_property_t property;
//    property.name = "Abc";
//    property.plugin_inst = "TestPluginInstance";
//    property.type = "MAGIC";
//    property.type_inst = "TestTypeInstance";
//
//    redfish_resource_t resource;
//    resource.name = "ResourceName";
//
//    redfish_service_t service;
//    service.name = "localhost";
//
//    const char* json_text =
//        "["
//        "  { \"Abc\": 4567 }"
//        "]";
//    json_error_t error;
//    json_t* root = json_loads(json_text, 0, &error);
//
//    if (!root) return -1;
//
//    //redfish_process_payload_property(&property, root, &resource, &service);
//
//    json_decref(root);
//
//    value_list_t* v = redfish_test_get_last_dispatched_value_list();
//    EXPECT_EQ_INT(1, v->values_len);
//    EXPECT_EQ_STR("MAGIC", v->type);
//    EXPECT_EQ_INT(4567, v->values->derive);
//    EXPECT_EQ_STR("TestPluginInstance", v->plugin_instance);
//    EXPECT_EQ_STR("TestTypeInstance", v->type_instance);
//    EXPECT_EQ_STR("localhost", v->host);
//    EXPECT_EQ_STR("redfish", v->plugin);
//
//    return 0;
//}
//
///*******/
//
//DEF_TEST(json_get_string_1)
//{
//    const char *json_text = "{ \"MemberId\": \"1234\" }";
//
//    json_error_t error;
//    json_t *root = json_loads(json_text, 0, &error);
//
//    if (!root) return -1;
//
//    char str[20];
//    json_t* json = json_object_get(root, "MemberId");
//    /***/
//    redfish_json_get_string(str, sizeof(str), json);
//
//    json_decref(root);
//
//    EXPECT_EQ_STR("1234", str);
//
//    return 0;
//}
//
///*******/
//
//DEF_TEST(json_get_string_2)
//{
//    const char *json_text = "{ \"MemberId\": 9876 }";
//
//    json_error_t error;
//    json_t *root = json_loads(json_text, 0, &error);
//
//    if (!root) return -1;
//
//    char str[20];
//    json_t* json = json_object_get(root, "MemberId");
//    /***/
//    redfish_json_get_string(str, sizeof(str), json);
//
//    json_decref(root);
//
//    EXPECT_EQ_STR("9876", str);
//
//    return 0;
//}

/*******/

int main(void)
{
    /* Building the in-memory version of the configuration file: */
    if (EXIT_FAILURE == build_config_file())
    {
        destroy_config_file();
        return EXIT_FAILURE;
    }

#if defined(COLLECT_DEBUG) && defined(REDFISH_TEST_PRINT_CONFIG)
    /* If the dedicated preprocessing variable was defined, printing the
     * configuration tree, notably for debug purposes: */
    oconfig_print_tree(
        config_file,
        OCONFIG_PRINT_TREE_INDENT_MAX_LVL,
        OCONFIG_PRINT_TREE_INDENT_IN_SPACES,
        stderr
    );
#endif

    /* Running the tests: */
    RUN_TEST(convert_val);
    RUN_TEST(redfish_preconfig);
    //RUN_TEST(read_queries);
    //RUN_TEST(config_property);
    //RUN_TEST(config_resource);
    //RUN_TEST(config_query);
    //RUN_TEST(config_service);
    //RUN_TEST(process_payload_property);
    //RUN_TEST(json_get_string_1);
    //RUN_TEST(json_get_string_2);

    /* Destroying the in-memory version of the configuration file: */
    destroy_config_file();

    /* Termination and summary of the test suite: */
    END_TEST;
}
