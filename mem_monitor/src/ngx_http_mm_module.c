#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include "mem_monitor.h"

/*
 * mem_monitor module is simple nginx http module, which purpose is to show  * how to use a timer in nginx.
 */

typedef struct
{
	ngx_int_t			mm_time;   			// in ms
	ngx_int_t			mm_event_timeout;   // in ms
	ngx_event_t			mm_event;  			// mm timer event
	ngx_int_t			is_header_sent;
	ngx_buf_t			*b;
	ngx_http_request_t 	*r;        			// current request
} ngx_http_mm_ctx_t;

static char *ngx_http_mm(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_mm_handler(ngx_http_request_t *r);
static void mm_event_handler(ngx_event_t *ev);

static ngx_command_t  ngx_http_mm_commands[] =
{
    {
        ngx_string("mm"),
        NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF |
        NGX_HTTP_LMT_CONF | NGX_CONF_NOARGS,
        ngx_http_mm,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL
    },

    ngx_null_command
};

static ngx_http_module_t  ngx_http_mm_module_ctx =
{
    NULL,                               /* preconfiguration */
    NULL,                  		        /* postconfiguration */

    NULL,                               /* create main configuration */
    NULL,                               /* init main configuration */

    NULL,                               /* create server configuration */
    NULL,                               /* merge server configuration */

    NULL,   							/* create location configuration */
    NULL     							/* merge location configuration */
};

ngx_module_t  ngx_http_mm_module =
{
    NGX_MODULE_V1,
    &ngx_http_mm_module_ctx,    /* module context */
    ngx_http_mm_commands,       /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};

static char *ngx_http_mm(ngx_conf_t *cf, ngx_command_t *cmd, 
	void *conf)
{
    ngx_http_core_loc_conf_t  *clcf;
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_mm_handler;
    return NGX_CONF_OK;
}

static ngx_int_t ngx_http_mm_handler(ngx_http_request_t *r)
{
    ngx_http_mm_ctx_t* ctx =
		ngx_http_get_module_ctx(r, ngx_http_mm_module);
    if (ctx == NULL){
        ctx = ngx_pcalloc(r->pool, sizeof(ngx_http_mm_ctx_t));
        if (ctx == NULL){
            return NGX_ERROR;
        }
        ngx_http_set_ctx(r, ctx, ngx_http_mm_module);

        ctx->mm_time = 10*1000;
        ctx->r = r;

		ctx->b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
		if (NULL == ctx->b) {
			return NGX_ERROR;
		}

        // set timer
        ctx->mm_event.log = r->connection->log;
        ctx->mm_event.data = ctx;
        ctx->mm_event.handler = mm_event_handler;
        ctx->mm_event_timeout = 1000;

		ngx_event_add_timer(&ctx->mm_event, 0);
		r->main->count++;
		return NGX_DONE;
    }
	return NGX_ERROR;
}

static void mm_event_handler(ngx_event_t *ev){
	ngx_http_mm_ctx_t *ctx;
	ngx_http_request_t *r;
	ngx_int_t rc;
	
	ctx = ev->data;
	r = ctx->r;

	if (0 == ctx->is_header_sent) {
    	// send header
		r->headers_out.content_type.len =sizeof("text/plain")-1;
		r->headers_out.content_type.data=(u_char *) "text/plain";
		r->headers_out.status=NGX_HTTP_OK;
		//r->headers_out.content_length_n=sizeof(content)-1;
		rc = ngx_http_send_header(r);
		if (rc != NGX_OK) {
			ngx_http_finalize_request(r, NGX_ERROR);
			return;
		}
		ctx->is_header_sent = 1;
	}

	ngx_chain_t out;
	u_char *content = ngx_pcalloc(r->pool, 256);
	rc = get_sys_free_mem((char*)content, 256);
	if (rc != 0) {
		ngx_http_finalize_request(r, NGX_ERROR);
		return;
	}

	ctx->b->pos = content;
	ctx->b->last = content + strlen((char*)content);
	ctx->b->memory = 1;
	ctx->b->flush = 1;
	ctx->b->last_buf = 0;
		
	out.buf = ctx->b;
	out.next = NULL;
	ngx_http_output_filter(r, &out);

	ctx->mm_time -= ctx->mm_event_timeout;

	if (ctx->mm_time <= 0) {
		ngx_http_finalize_request(r, 
			ngx_http_send_special(r, NGX_HTTP_LAST));
	} else {
		ngx_event_add_timer(&ctx->mm_event, ctx->mm_event_timeout);
	}
	return;
}
