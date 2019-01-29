#include <fcntl.h>
#include <resolv.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#include <uci.h>
#include <uci_blob.h>

#include "asrock_mqttd.h"
#include "log.h"

static struct blob_buf b;
static int reload_pipe[2];
struct list_head irs = LIST_HEAD_INIT(irs);
struct config config = {false, NULL, NULL};
int ir_index = 0;

enum {
	IR_ATTR_ENABLE,
	IR_ATTR_NAME,
	IR_ATTR_VALUE,
	IR_ATTR_MAX
};


static const struct blobmsg_policy ir_attrs[IR_ATTR_MAX] = {
	[IR_ATTR_ENABLE] = { .name = "enable", .type = BLOBMSG_TYPE_BOOL },
	[IR_ATTR_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
	[IR_ATTR_VALUE] = { .name = "value", .type = BLOBMSG_TYPE_STRING },
};


const struct uci_blob_param_list ir_attr_list = {
	.n_params = IR_ATTR_MAX,
	.params = ir_attrs,
};

static int set_ir(struct uci_section *s) {
	
	struct blob_attr *tb[IR_ATTR_MAX], *c;
	blob_buf_init(&b, 0);
	uci_to_blob(&b, s, &ir_attr_list);
	blobmsg_parse(ir_attrs, IR_ATTR_MAX, tb, blob_data(b.head), blob_len(b.head));

	size_t valuelen = 0;
	if ((c = tb[IR_ATTR_VALUE]))
		valuelen = blobmsg_data_len(c);

	struct ir *ir = calloc(1, (sizeof(*ir) + valuelen));
#if DEBUG_VERBOSE
	DEBUG("sizeof(*ir)= %d, valuelen=%d",sizeof(*ir), valuelen);
#endif
	if (!ir)
		goto err;

	ir->channel_id = ir_index;
#if DEBUG_VERBOSE
	DEBUG("ir[%d]->index %d", ir->channel_id, ir->channel_id);
#endif

	if ((c = tb[IR_ATTR_ENABLE])) {
		ir->enable = blobmsg_get_bool(c);
#if DEBUG_VERBOSE
		DEBUG("ir[%d]->enable %d", ir->channel_id, ir->enable);
#endif
	}

	if ((c = tb[IR_ATTR_NAME])) {
		strcpy(ir->name, blobmsg_get_string(c));
#if DEBUG_VERBOSE
		DEBUG("ir[%d]->name %s", ir->channel_id, ir->name);
#endif
	}

	if ((c = tb[IR_ATTR_VALUE])) {
		memcpy(ir->value, blobmsg_get_string(c), valuelen);
#if DEBUG_VERBOSE
		DEBUG("ir[%d]->value %s", ir->channel_id, ir->value);
#endif
	}

	list_add(&ir->head, &irs);

	ir_index++;

	return 0;

err:
	if (ir) {
		free(ir);
	}
	return -1;
}

enum {
	ASROCK_ATTR_ENABLE,
	ASROCK_ATTR_DEVICE_KEY,
	ASROCK_ATTR_SSID,
	ASROCK_ATTR_USERNAME,
	ASROCK_ATTR_PASSWORD,
	ASROCK_ATTR_MAX
};

static const struct blobmsg_policy asrock_attrs[ASROCK_ATTR_MAX] = {
	[ASROCK_ATTR_ENABLE] = { .name = "enable", .type = BLOBMSG_TYPE_BOOL },
	[ASROCK_ATTR_DEVICE_KEY] = { .name = "device_key", .type = BLOBMSG_TYPE_STRING },
	[ASROCK_ATTR_SSID] = { .name = "ssid", .type = BLOBMSG_TYPE_STRING },
	[ASROCK_ATTR_USERNAME] = { .name = "username", .type = BLOBMSG_TYPE_STRING },
	[ASROCK_ATTR_PASSWORD] = { .name = "password", .type = BLOBMSG_TYPE_STRING },
};

const struct uci_blob_param_list asrock_attr_list = {
	.n_params = ASROCK_ATTR_MAX,
	.params = asrock_attrs,
};

static int set_asrock(struct uci_section *s) {

	int result=0, count=0;
	struct blob_attr *tb[ASROCK_ATTR_MAX], *c;

	blob_buf_init(&b, 0);
	uci_to_blob(&b, s, &asrock_attr_list);
	blobmsg_parse(asrock_attrs, ASROCK_ATTR_MAX, tb, blob_data(b.head), blob_len(b.head));

	if ((c = tb[ASROCK_ATTR_ENABLE])) {
		config.enable = blobmsg_get_bool(c);
		count++;
	}

	if ((c = tb[ASROCK_ATTR_DEVICE_KEY])) {
		free(config.device_key);
		config.device_key = strdup(blobmsg_get_string(c));
		count++;
	}

	if ((c = tb[ASROCK_ATTR_SSID])) {
		free(config.asrock_ssid);
		config.asrock_ssid = strdup(blobmsg_get_string(c));
		count++;
	}

	if ((c = tb[ASROCK_ATTR_USERNAME])) {
		free(config.username);
		config.username = strdup(blobmsg_get_string(c));
		count++;
	}

	if ((c = tb[ASROCK_ATTR_PASSWORD])) {
		free(config.password);
		config.password = strdup(blobmsg_get_string(c));
		count++;
	}

	if(count==5) {
		result = 1;
#if DEBUG_VERBOSE
		DEBUG("count %d", count);
#endif
	}
	return result;
}

int asrock_mqttd_reload(void) {
	int result = 0;
	ir_index = 0 ;
	struct uci_context *uci = uci_alloc_context();

	if (!uci)
		return result;

	struct uci_package *asrock = NULL;
	if (!uci_load(uci, "asrock", &asrock)) {
		struct uci_element *e;
		uci_foreach_element(&asrock->sections, e) {
			struct uci_section *s = uci_to_section(e);
			if (!strcmp(s->type, "asrock_mqttd")) {
				result = set_asrock(s);
			} /*else if (!strcmp(s->type, "ir")) {
				//set_ir(s);
			}*/
		}

	}

	uci_unload(uci, asrock);

	while (!list_empty(&irs)) {
		struct ir *l = list_first_entry(&irs, struct ir, head);
		list_del(&l->head);
		free(l);
	}

	struct uci_package *ir = NULL;
	if (!uci_load(uci, "ir", &ir)) {
		struct uci_element *e;
		uci_foreach_element(&ir->sections, e) {
			struct uci_section *s = uci_to_section(e);
			if (!strcmp(s->type, "keycode")) {
				set_ir(s);
			}
		}

	}

	uci_unload(uci, ir);
	uci_free_context(uci);
	
#if DEBUG_VERBOSE
	struct ir *ir_debug;
	list_for_each_entry(ir_debug, &irs, head) {
		DEBUG("ir->enable %d", ir_debug->enable);
		
 	}
#endif

	return result;
}


static void handle_signal(int signal) {

	char b[1] = {0};
#if DEBUG_VERBOSE
		DEBUG("handle_signal[%d]", signal);
#endif
	if (signal == SIGHUP) {
		if (write(reload_pipe[1], b, sizeof(b)) < 0) {}
	} else
		uloop_end();
}


static void reload_cb(struct uloop_fd *u, _unused unsigned int events) {

	char b[512];
	if (read(u->fd, b, sizeof(b)) < 0) {}
	asrock_mqttd_reload();
}

static struct uloop_fd reload_fd = { .cb = reload_cb };

void asrock_mqttd_run(void) {

	if (pipe2(reload_pipe, O_NONBLOCK | O_CLOEXEC) < 0) {}
	reload_fd.fd = reload_pipe[0];
	uloop_fd_add(&reload_fd, ULOOP_READ);

	signal(SIGTERM, handle_signal);
	signal(SIGINT, handle_signal);
	signal(SIGHUP, handle_signal);

	if(!asrock_mqttd_reload()) {
#if DEBUG_VERBOSE
		DEBUG("Failed to acqurie data uci...");
#endif
		exit(1);
	}

	asrock_main();

	uloop_run();

}

