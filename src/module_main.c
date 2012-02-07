/*
 * Copyright (c) Maxwell Dayvson <dayvson@gmail.com>
 * Copyright (c) Tiago de Pádua <tiagopadua@gmail.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "httpd.h"
#include "http_config.h"
#include "ap_config.h"
#include "thumbnailer.h"

// Just define the module, for references.
// Go to the last lines to see its value
module AP_MODULE_DECLARE_DATA videothumb_module;

static int module_handler(request_rec *r) {
   // Get the module configuration
   //orchid_config* o_cfg = ap_get_module_config(r->server->module_config, &orchid_module);

//   LOG_ERROR("enabled........ %s\n", o_cfg->enabled);
//   LOG_ERROR("password....... %s\n", o_cfg->password);
//   LOG_ERROR("timeout........ %s\n", o_cfg->timeout);
//   LOG_ERROR("validate_ip.... %s\n", o_cfg->validate_ip);

//   LOG_ERROR("uri............ %s\n", r->uri);
//   LOG_ERROR("unparsed_uri... %s\n", r->unparsed_uri);
//   LOG_ERROR("args........... %s\n", r->args);
//   LOG_ERROR("hostname....... %s\n", r->hostname);
//   LOG_ERROR("ip............. %s\n", r->connection->remote_ip);

	//return DECLINED;

   // "DECLINED" means that the module will not change 
   //  this request - and Apache must proceed normally
   //if (req_result == ORCHID_SUCCESS) return DECLINED; 
   //else if (req_result == ORCHID_FAIL) return HTTP_NOT_FOUND;
   //else if (req_result == ORCHID_MONIT) return ORCHID_HTTP_MONIT;

// Do not use fflush so often on a production server
//   fflush(stderr);

   // If anything is unexpected,
   // Consider by default the module is enabled and the request was invalid
	if (tve_open_video("/Users/tiagopadua/Movies/cartoons.avi") == 0)
		return DECLINED;
	else
   		return HTTP_NOT_FOUND;
}

static void register_hooks (apr_pool_t *p) {
   ap_hook_post_read_request(module_handler, NULL, NULL, APR_HOOK_FIRST);
}

/************************************************************
 *  The name of this structure is important - it must match *
 *  the name of the module.  This structure is the          *
 *  only "glue" between the httpd core and the module.      *
 ************************************************************/
module AP_MODULE_DECLARE_DATA videothumb_module = {
      STANDARD20_MODULE_STUFF,
      NULL,
      NULL,
      NULL, //orchid_create_config,
      NULL,
      NULL, //orchid_config_cmds,
      register_hooks
};
