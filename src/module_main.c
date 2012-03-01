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

#include "module.h"
#include "log.h"
#include "thumbnailer.h"

#define CONFIG_DEFAULT_ENABLED "enabled"

static int module_handler(request_rec *r) {
  if (r->args) {
    LOG_ERROR("request made: %s", r->args);
    tve_init_libraries();
    LOG_ERROR("blabalbal");
    ImageBuffer jpeg = tve_open_video(r->args, 5);
    
    if (jpeg.buffer) {
      LOG_ERROR(">> Retornando JPEG");
      ap_set_content_type(r, "image/jpeg");
      LOG_ERROR(">>> mbmabkfmdakba");
      ap_rwrite(jpeg.buffer, jpeg.size, r);
      // ap_rputs(jpeg, jpeg.size, r);
      LOG_ERROR("depois do rputs");
      } else LOG_ERROR("JPEG INVALIDO!!!!!!!!!!!!!!!!");
  } else {
    LOG_ERROR("Querystring null");
  }
  return OK;
  // else
    // return HTTP_NOT_FOUND;
  /*  fprintf(stderr, "args........... %s\n", r->args);

    tve_init_libraries();
  	if (tve_open_video("/Users/dayvson/Apache-Get-Video-Thumbnail/src/daisy.mp4", 25) == 0)
  		return DECLINED;
  	else
     		return HTTP_NOT_FOUND;
  */
}

static void register_hooks (apr_pool_t *p) {
   ap_hook_handler(module_handler, NULL, NULL, APR_HOOK_LAST);
}

// Set up the configuration
typedef struct
{
  char* enabled;
} module_config;

static void* create_config(apr_pool_t* p, server_rec* r)
{
  // Allocate memory for the config object
  // Using 'apr_pool' we don't have to worry about DE-allocating memory - apache does it
  module_config* newcfg = (module_config*)apr_pcalloc(p, sizeof(module_config));

  // Set default values
  newcfg->enabled = CONFIG_DEFAULT_ENABLED;
}

static const char* config_set_enabled(cmd_parms* parms, void* mconfig, const char* arg)
{
  module_config* cfg = ap_get_module_config(parms->server->module_config, &videothumb_module);
  cfg->enabled = (char*)arg;
  return NULL;
}

static const command_rec config_array[] =
{
  AP_INIT_TAKE1(
    "VideothumbEnabled",
    config_set_enabled,
    NULL,
    RSRC_CONF,
    "Orchid configuration key loaded: VideothumbEnabled"),
  { NULL }
};

/************************************************************
 *  The name of this structure is important - it must match *
 *  the name of the module.  This structure is the          *
 *  only "glue" between the httpd core and the module.      *
 ************************************************************/
module AP_MODULE_DECLARE_DATA videothumb_module = {
      STANDARD20_MODULE_STUFF,
      NULL,
      NULL,
      create_config,
      NULL,
      config_array,
      register_hooks
};
