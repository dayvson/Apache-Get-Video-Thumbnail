/*
 * Copyright (c) 2012 Maxwell Dayvson <dayvson@gmail.com>
 * Copyright (c) 2012 Tiago de Pádua <tiagopadua@gmail.com>
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

#include "video_thumbnail_module.h"
#include "storyboard.h"
#include "thumbnail.h"
#define BOOL int

#define ONE_VALUE "1"
#define CONFIG_DEFAULT_ENABLED FALSE
#define CONFIG_DEFAULT_MEDIAS_PATH "/tmp"
#define CONFIG_DEFAULT_JPEG_QUALITY 70

#define MAX_PATH_LENGTH 1024

// Set up the configuration
typedef struct
{
  BOOL enabled;
  const char* medias_path;
  const char* app_path;
  int quality;
} module_config;

static int videothumb_handler(request_rec *r) 
{
  char fullVideoPath[MAX_PATH_LENGTH];
  
  if (!r) return DECLINED;
  module_config* conf = ap_get_module_config(r->server->module_config, &videothumb_module);

  if (!conf->enabled) 
  {
//    LOG_ERROR("VideoThumb module is disabled. Returning request to apache...");
    return DECLINED;
  }
  if (!(conf->app_path && (strstr(r->parsed_uri.path, conf->app_path) == r->parsed_uri.path)))
  {
//    LOG_ERROR(">>>>>>>>>>>>>>>>>>> %s || %s", conf->app_path, r->parsed_uri.path);
//    LOG_ERROR(">>>>>>>>>>>>>>>>>>> %d", strstr(r->parsed_uri.path, conf->app_path) == r->parsed_uri.path);
    return DECLINED;
  }

  if (r->args) 
  {
    LOG_ERROR("Apache-Get-Video-Thumbnail acting");
//    LOG_ERROR(">>>>>>>>>>>>>>>>>>>>> %s", r->unparsed_uri);
//    LOG_ERROR(">>>>>>>>>>>>>>>>>>>>> %s", r->uri);
//    LOG_ERROR(">>>>>>>>>>>>>>>>>>>>> %s", r->path_info);
//    LOG_ERROR(">>>>>>>>>>>>>>>>>>>>> %s", r->filename);
//    LOG_ERROR(">>>>>>>>>>>>>>>>>>>>> %s", r->canonical_filename);

    void* ctx;
    parse_query_string(&ctx, r->args);
    strncpy(fullVideoPath, conf->medias_path, MAX_PATH_LENGTH);
//    strncat(fullVideoPath, get_parameter(ctx, "video"), MAX_PATH_LENGTH);
    if (!r->path_info) {
       return DECLINED;
    }
    strncat(fullVideoPath, r->path_info + 1, MAX_PATH_LENGTH);

    const char* temp = get_parameter(ctx, "split");
    RequestInfo requestInfo;
    requestInfo.split = atoi(temp ? temp : ONE_VALUE);
    temp = get_parameter(ctx, "columns");
    requestInfo.columns = atoi(temp ? temp : ONE_VALUE);
    requestInfo.file = fullVideoPath;
    requestInfo.jpegQuality = conf->quality;
  
    temp = get_parameter(ctx,"width");
    if(temp == NULL) requestInfo.width = 0;
    else requestInfo.width = atoi(temp);
  
    temp = get_parameter(ctx,"height");
    if(temp == NULL) requestInfo.height = 0;
    else requestInfo.height = atoi(temp);

    temp = get_parameter(ctx,"pageSize");
    requestInfo.pageSize = temp ? atoi(temp) : requestInfo.split;

    temp = get_parameter(ctx,"currentPage");
    requestInfo.currentPage = atoi(temp ? temp : ONE_VALUE);

    init_libraries();
    temp = get_parameter(ctx, "second");
    ImageBuffer jpeg;
    if(temp == NULL) jpeg = get_storyboard(requestInfo);
    else {
      requestInfo.second =  atoi(temp ? temp : ONE_VALUE);
      jpeg = get_thumbnail(requestInfo);
    }
  
    if (jpeg.buffer) {
      ap_set_content_type(r, "image/jpeg");
      ap_rwrite(jpeg.buffer, jpeg.size, r);
      } else {
        LOG_ERROR("JPEG INVALIDO!!!!!!!!!!!!!!!!");
      }
  } else {
    LOG_ERROR("Querystring null");
  }
  return OK;
}

static void register_hooks (apr_pool_t *p) 
{
   ap_hook_handler(videothumb_handler, NULL, NULL, APR_HOOK_LAST);
}

static void* create_config(apr_pool_t* p, server_rec* r)
{
  // Allocate memory for the config object
  // Using 'apr_pool' we don't have to worry about DE-allocating memory - apache does it
  module_config* newcfg = (module_config*)apr_pcalloc(p, sizeof(module_config));

  // Set default values
  newcfg->enabled = CONFIG_DEFAULT_ENABLED;
  newcfg->medias_path = CONFIG_DEFAULT_MEDIAS_PATH;
  newcfg->quality = CONFIG_DEFAULT_JPEG_QUALITY;

  return newcfg;
}

static const char* config_set_enabled(cmd_parms* parms, void* mconfig, const char* arg)
{
  module_config* cfg = ap_get_module_config(parms->server->module_config, &videothumb_module);
  cfg->enabled = strcmp(arg, "true") == 0;
  return NULL;
}
static const char* config_set_medias_path(cmd_parms* parms, void* mconfig, const char* arg)
{
  module_config* cfg = ap_get_module_config(parms->server->module_config, &videothumb_module);
  cfg->medias_path = (char*)arg;
  return NULL;
}
static const char* config_set_quality(cmd_parms* parms, void* mconfig, const char* arg)
{
  module_config* cfg = ap_get_module_config(parms->server->module_config, &videothumb_module);
  cfg->quality = atoi(arg);
  return NULL;
}
static const char* config_set_app_path(cmd_parms* parms, void* mconfig, const char* arg)
{
  module_config* cfg = ap_get_module_config(parms->server->module_config, &videothumb_module);
  cfg->app_path = arg;
  return NULL;
}

static const command_rec config_array[] =
{
  AP_INIT_TAKE1(
    "VideoThumb_Enabled",
    config_set_enabled,
    NULL,
    RSRC_CONF,
    "Videothumb configuration key loaded: VideoThumb_Enabled"),
  AP_INIT_TAKE1(
    "VideoThumb_MediasPath",
    config_set_medias_path,
    NULL,
    RSRC_CONF,
    "Videothumb configuration key loaded: VideoThumb_MediasPath"),
  AP_INIT_TAKE1(
    "VideoThumb_AppPath",
    config_set_app_path,
    NULL,
    RSRC_CONF,
    "Videothumb configuration key loaded: VideoThumb_AppPath"),
  AP_INIT_TAKE1(
    "VideoThumb_JpegQuality",
    config_set_quality,
    NULL,
    RSRC_CONF,
    "Videothumb configuration key loaded: VideoThumb_JpegQuality"),
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
