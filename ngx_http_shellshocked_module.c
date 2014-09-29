#include <nginx.h>
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#define TRUE 1
#define FALSE 0

ngx_module_t ngx_http_shellshocked_module;


typedef struct {
  ngx_flag_t enabled;
} shellshocked_main_conf_t;


static ngx_int_t
shellshocked_payload_present(ngx_table_elt_t *h)
{
  if (memcmp(h->value.data, "() {", 4) == 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}


static ngx_int_t
check_headers(ngx_http_request_t *r)
{
  ngx_list_part_t *part;
  ngx_table_elt_t *h;
  ngx_uint_t i;

  part = &r->headers_in.headers.part;
  h = part->elts;

  for (i = 0; /**/; i++) {
    if (i >= part->nelts) {
      if (part->next == NULL) {
        break;
      }

      part = part->next;
      h = part->elts;
      i = 0;
    }

    if (shellshocked_payload_present(&h[i])) {
      ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
		    "[shellshocked] blocking header %s with value: %s",
		    h[i].key.data, h[i].value.data);
      return NGX_ERROR;
    }
  }

  return NGX_DECLINED;
}


static ngx_int_t
ngx_http_shellshocked_handler(ngx_http_request_t *r)
{
  shellshocked_main_conf_t *conf = ngx_http_get_module_main_conf(r, ngx_http_shellshocked_module);

  if (!conf->enabled || conf->enabled == NGX_CONF_UNSET || r->main->internal) {
    return NGX_DECLINED;
  }

  ngx_int_t result = check_headers(r);
  if (result == NGX_ERROR) {
    return NGX_HTTP_FORBIDDEN;
  }

  return NGX_OK;
}


static ngx_int_t
ngx_http_shellshocked_init(ngx_conf_t *cf)
{
  ngx_http_handler_pt *h;
  ngx_http_core_main_conf_t *cmcf;

  cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);
  h = ngx_array_push(&cmcf->phases[NGX_HTTP_ACCESS_PHASE].handlers);

  if (h == NULL) {
    return NGX_ERROR;
  }

  *h = ngx_http_shellshocked_handler;

  return NGX_OK;
}


static void*
ngx_http_shellshocked_create_main_conf(ngx_conf_t *cf)
{
  shellshocked_main_conf_t *conf;

  conf = ngx_pcalloc(cf->pool, sizeof(shellshocked_main_conf_t));
  if (conf == NULL) {
    return NULL;
  }

  conf->enabled = NGX_CONF_UNSET;

  return conf;
}


static ngx_command_t ngx_http_shellshocked_commands[] = {
  {
    ngx_string("shellshocked"),
    NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
    ngx_conf_set_flag_slot,
    NGX_HTTP_MAIN_CONF_OFFSET,
    offsetof(shellshocked_main_conf_t, enabled),
    NULL
  },

  ngx_null_command
};


static ngx_http_module_t ngx_http_shellshocked_module_ctx = {
  NULL,                                   /* preconfiguration */
  ngx_http_shellshocked_init,             /* postconfiguration */
  ngx_http_shellshocked_create_main_conf, /* create main configuration */
  NULL,                                   /* init main configuration */
  NULL,                                   /* create server configuration */
  NULL,                                   /* merge server configuration */
  NULL,                                   /* create location configuration */
  NULL                                    /* merge location configuration */
};


ngx_module_t ngx_http_shellshocked_module = {
  NGX_MODULE_V1,
  &ngx_http_shellshocked_module_ctx,  /* module context */
  ngx_http_shellshocked_commands,     /* module directives */
  NGX_HTTP_MODULE,                    /* module type */
  NULL,                               /* init master */
  NULL,                               /* init module */
  NULL,                               /* init process */
  NULL,                               /* init thread */
  NULL,                               /* exit thread */
  NULL,                               /* exit process */
  NULL,                               /* exit master */
  NGX_MODULE_V1_PADDING
};
