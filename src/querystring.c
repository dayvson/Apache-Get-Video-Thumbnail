/*
 * Copyright (c) 2012 - Maxwell Dayvson <dayvson@gmail.com>
 * Copyright (c) 2012 - Tiago de Pádua <tiagopadua@gmail.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "querystring.h"

#define INITIAL_MAX_PARAMETER_COUNT 50

void* create_new_context(char** items, int count)
{
  if (!items) return NULL;
  
  QueryString *query = (QueryString*)malloc(sizeof(QueryString));
  query->Item = (NameValuePair**)malloc(sizeof(NameValuePair*) * count);
  query->count = 0;

  int i=0;
  for (i=0; i<count; ++i) {
    if (!items[i]) continue;
    char* chPos = strchr(items[i], '=');
    query->Item[i] = (NameValuePair*)malloc(sizeof(NameValuePair));
    if (chPos) {
      // Copy the name
      size_t nChars = ((size_t)chPos) - ((size_t)items[i]);
      query->Item[i]->name = (char*)malloc(sizeof(char) * (nChars+1));
      strncpy(query->Item[i]->name, items[i], nChars);
      query->Item[i]->name[nChars] = '\0';
      // Copy the value
      ++chPos; // skip the '='
      query->Item[i]->value = (char*)malloc(sizeof(char) * strlen(chPos));
      strcpy(query->Item[i]->value, chPos);
    } else {
      // Copy the name
      query->Item[i]->name = (char*)malloc(sizeof(char) * strlen(items[i]));
      strcpy(query->Item[i]->name, items[i]);
      // Fill the value with an empty string
      query->Item[i]->value = (char*)malloc(sizeof(char));
      query->Item[i]->value[0] = '\0';
    }
    free(items[i]);
    ++query->count;
  }
  return query;
}

void release_context(void* context)
{
  if (!context) return;

  int i=0;
  QueryString* qs = (QueryString*)context;
  if (qs->Item) {
    for (i=0; i<qs->count; ++i) {
      if (qs->Item[i])
      {
        if (qs->Item[i]->name) free(qs->Item[i]->name);
        if (qs->Item[i]->value) free(qs->Item[i]->value);
        free(qs->Item[i]);
      }
    }
    free(qs->Item);
    qs->Item = NULL;
  }
  free(qs);
}

int split_parameters(char*** items, const char* querystring)
{
  int count = 0;

  (*items) = (char**)malloc(sizeof(char*) * INITIAL_MAX_PARAMETER_COUNT);
  if (querystring) {
    const char* oldPos = querystring;
    char* nextPos = strchr(querystring, '&');
    while (nextPos) {
      // Prevent for filling null pointers
      if (count >= INITIAL_MAX_PARAMETER_COUNT) break;

      size_t strSize = nextPos - oldPos;
      char *currentParam = (char*)malloc(strSize+1);
      strncpy(currentParam, oldPos, strSize);
      currentParam[strSize] = '\0';
      (*items)[count] = currentParam;

      ++count;
      oldPos = nextPos + 1;
      nextPos = strchr(oldPos, '&');
    }
    // fill the last object
    size_t strSize = strlen(oldPos);
    (*items)[count] = (char*)malloc(strSize+1);
    strncpy((*items)[count], oldPos, strSize);
    (*items)[count][strSize] = '\0';
    ++count;
  }
  return count;
}

int parse_query_string(void** context, const char* querystring)
{
  char** items = NULL;

  if (querystring) {
    int itemCount = split_parameters(&items, querystring);
    (*context) = create_new_context(items, itemCount);    
    free(items);
  }

  return 0;
}

const char* get_parameter(void* context, const char* parName)
{
  if (!context || !parName) return NULL;

  int i=0;
  QueryString* qs = (QueryString*)context;
  for (i=0; i<qs->count; ++i) {
    if (0 == strcmp(parName, qs->Item[i]->name)) {
      return qs->Item[i]->value;
    }
  }
  return NULL;
}
