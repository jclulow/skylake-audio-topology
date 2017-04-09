

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <err.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "custr.h"

#include "../from_3.18/asoc.h"
#include "../from_3.18/skl-tplg-interface.h"


const char *type_names[SND_SOC_TPLG_TYPE_MAX + 1] = {
	[SND_SOC_TPLG_TYPE_MIXER] = "MIXER",
	[SND_SOC_TPLG_TYPE_BYTES] = "BYTES",
	[SND_SOC_TPLG_TYPE_ENUM] = "ENUM",
	[SND_SOC_TPLG_TYPE_DAPM_GRAPH] = "DAPM_GRAPH",
	[SND_SOC_TPLG_TYPE_DAPM_WIDGET] = "DAPM_WIDGET",
	[SND_SOC_TPLG_TYPE_DAI_LINK] = "DAI_LINK",
	[SND_SOC_TPLG_TYPE_PCM] = "PCM",
	[SND_SOC_TPLG_TYPE_MANIFEST] = "MANIFEST",
	[SND_SOC_TPLG_TYPE_CODEC_LINK] = "CODEC_LINK",
	[SND_SOC_TPLG_TYPE_BACKEND_LINK] = "BACKEND_LINK",
	[SND_SOC_TPLG_TYPE_PDATA] = "PDATA",
};

const char *widget_id_names[SND_SOC_TPLG_DAPM_LAST + 1] = {
	[SND_SOC_TPLG_DAPM_INPUT] = "input",
	[SND_SOC_TPLG_DAPM_OUTPUT] = "output",
	[SND_SOC_TPLG_DAPM_MUX] = "mux",
	[SND_SOC_TPLG_DAPM_MIXER] = "mixer",
	[SND_SOC_TPLG_DAPM_PGA] = "pga",
	[SND_SOC_TPLG_DAPM_OUT_DRV] = "out_drv",
	[SND_SOC_TPLG_DAPM_ADC] = "adc",
	[SND_SOC_TPLG_DAPM_DAC] = "dac",
	[SND_SOC_TPLG_DAPM_SWITCH] = "switch",
	[SND_SOC_TPLG_DAPM_PRE] = "pre",
	[SND_SOC_TPLG_DAPM_POST] = "post",
	[SND_SOC_TPLG_DAPM_AIF_IN] = "aif_in",
	[SND_SOC_TPLG_DAPM_AIF_OUT] = "aif_out",
	[SND_SOC_TPLG_DAPM_DAI_IN] = "dai_in",
	[SND_SOC_TPLG_DAPM_DAI_OUT] = "dai_out",
	[SND_SOC_TPLG_DAPM_DAI_LINK] = "dai_link",
};

const char *kc_info_op_names[256] = {
	[SND_SOC_TPLG_CTL_VOLSW] = "VOLSW",
	[SND_SOC_TPLG_CTL_VOLSW_SX] = "VOLSW_SX",
	[SND_SOC_TPLG_CTL_VOLSW_XR_SX] = "VOLSW_XR_SX",
	[SND_SOC_TPLG_CTL_ENUM] = "ENUM",
	[SND_SOC_TPLG_CTL_BYTES] = "BYTES",
	[SND_SOC_TPLG_CTL_ENUM_VALUE] = "ENUM_VALUE",
	[SND_SOC_TPLG_CTL_RANGE] = "RANGE",
	[SND_SOC_TPLG_CTL_STROBE] = "STROBE",

	[SND_SOC_TPLG_DAPM_CTL_VOLSW] = "DAPM VOLSW",
	[SND_SOC_TPLG_DAPM_CTL_ENUM_DOUBLE] = "DAPM ENUM_DOUBLE",
	[SND_SOC_TPLG_DAPM_CTL_ENUM_VIRT] = "DAPM ENUM_VIRT",
	[SND_SOC_TPLG_DAPM_CTL_ENUM_VALUE] = "DAPM ENUM_VALUE",
	[SND_SOC_TPLG_DAPM_CTL_PIN] = "DAPM PIN",
};



void *data = NULL;

custr_t *s_tuples;
custr_t *s_data;
custr_t *s_mixers;
custr_t *s_widgets;
custr_t *s_graph;

static void
print_header(struct snd_soc_tplg_hdr *hdr)
{
	fprintf(stderr, "HEADER %p (offset %u)\n", (void *)hdr,
	    (unsigned)((void *)hdr - data));

	char mag[4];
	memcpy(mag, &hdr->magic, 4);
	fprintf(stderr, "\tMAGIC\t\t%c%c%c%c\n", mag[3],
	    mag[2], mag[1], mag[0]);

	fprintf(stderr, "\tABI\t\t%u\n", hdr->abi);

	fprintf(stderr, "\tVERSION\t\t%u\n", hdr->version);

	const char *tn = "<UNKNOWN>";
	if (hdr->type <= SND_SOC_TPLG_TYPE_MAX)
		tn = type_names[hdr->type];
	fprintf(stderr, "\tTYPE\t\t%s (%u)\n", tn, hdr->type);

	fprintf(stderr, "\tHEADER SIZE\t%u\n", hdr->size);

	fprintf(stderr, "\tVENDOR TYPE\t%u\n", hdr->vendor_type);

	fprintf(stderr, "\tPAYLOAD SIZE\t%u\n", hdr->payload_size);

	fprintf(stderr, "\tINDEX\t\t%u\n", hdr->index);

	fprintf(stderr, "\tCOUNT\t\t%u\n", hdr->count);
}

static void
print_manifest(struct snd_soc_tplg_manifest *manifest)
{
	fprintf(stderr, "-- MANIFEST ----------------------------\n");
	fprintf(stderr, "\tSIZE\t\t%u\n", manifest->size);
	fprintf(stderr, "\t#CONTROL\t%u\n", manifest->control_elems);
	fprintf(stderr, "\t#WIDGET\t\t%u\n", manifest->widget_elems);
	fprintf(stderr, "\t#GRAPH\t\t%u\n", manifest->graph_elems);
	fprintf(stderr, "\t#DAI\t\t%u\n", manifest->dai_elems);
	fprintf(stderr, "\t#DAI LINK\t%u\n", manifest->dai_link_elems);
	fprintf(stderr, "\n");
	fprintf(stderr, "\tPRIV DATA SIZE\t%u\n", manifest->priv.size);
}

static int
lenchk(char *x, size_t maxlen)
{
	return (strnlen(x, maxlen) < maxlen);
}

static void
print_graph(struct snd_soc_tplg_hdr *hdr, void *payload, unsigned nelems)
{
	void *pos = payload;
	unsigned i = 0;

	custr_append_printf(s_graph, "SectionGraph.\"Pipeline %u Graph\" {\n",
	    hdr->index + 1);
	custr_append_printf(s_graph, "\tindex\"%u\"\n", hdr->index + 1);
	custr_append_printf(s_graph, "\tlines [\n");

	while (i < nelems) {
		struct snd_soc_tplg_dapm_graph_elem *g = pos;

#if 0
		fprintf(stderr, "-- GRAPH ELEMENT %u ----------------------\n",
		    i);
#endif

		if (!lenchk(g->sink, SNDRV_CTL_ELEM_ID_NAME_MAXLEN) ||
		    !lenchk(g->control, SNDRV_CTL_ELEM_ID_NAME_MAXLEN) ||
		    !lenchk(g->source, SNDRV_CTL_ELEM_ID_NAME_MAXLEN)) {
			fprintf(stderr, "WARNING: invalid string\n");
			return;
		}

		custr_append_printf(s_graph, "\t\t\"%s, %s, %s\"\n",
		    g->sink, g->control, g->source);

#if 0
		fprintf(stderr, "\tSINK\t\t\"%s\"\n", g->sink);
		fprintf(stderr, "\tCONTROL\t\t\"%s\"\n", g->control);
		fprintf(stderr, "\tSOURCE\t\t\"%s\"\n", g->source);
#endif

		pos += sizeof (*g);
		i++;
	}

	custr_append_printf(s_graph, "\t]\n");
	custr_append_printf(s_graph, "}\n");
}

static unsigned
val_at(const char *pair)
{
	char cpair[3];

	cpair[0] = pair[0];
	cpair[1] = pair[1];
	cpair[2] = '\0';

	unsigned out = strtoul(cpair, NULL, 16);

	return (out);
}

static void
emit_uuid_tuple(const char *uuid, custr_t *cu)
{
	/*
	 * e.g., "9BA00C83-CA12-4A83-943C-1FA2E82F9DDA"
	 */

	custr_append_printf(cu, "\t\tSKL_TKN_UUID \"");

	custr_append_printf(cu, "%u, ", val_at(uuid + 6));
	custr_append_printf(cu, "%u, ", val_at(uuid + 4));
	custr_append_printf(cu, "%u, ", val_at(uuid + 2));
	custr_append_printf(cu, "%u, ", val_at(uuid + 0));

	custr_append_printf(cu, "%u, ", val_at(uuid + 11));
	custr_append_printf(cu, "%u,\n", val_at(uuid + 9));

	custr_append_printf(cu, "\t\t");

	custr_append_printf(cu, "%u, ", val_at(uuid + 16));
	custr_append_printf(cu, "%u, ", val_at(uuid + 14));

	custr_append_printf(cu, "%u, ", val_at(uuid + 19));
	custr_append_printf(cu, "%u, ", val_at(uuid + 21));

	custr_append_printf(cu, "%u, ", val_at(uuid + 24));
	custr_append_printf(cu, "%u, ", val_at(uuid + 26));

	custr_append_printf(cu, "%u, ", val_at(uuid + 28));
	custr_append_printf(cu, "%u, ", val_at(uuid + 30));
	custr_append_printf(cu, "%u, ", val_at(uuid + 32));
	custr_append_printf(cu, "%u\"\n", val_at(uuid + 34));
}

#if 0
        uint32_t channels;
        uint32_t freq;
        uint32_t bit_depth;
        uint32_t valid_bit_depth;
        uint32_t ch_cfg;
        uint32_t interleaving_style;
        uint32_t sample_type;
        uint32_t ch_map;
#endif

static void
emit_format(struct skl_dfw_module_fmt *fmt, custr_t *cu, unsigned dir, unsigned idx)
{
	unsigned dpc = dir | (idx << 4);

	custr_append_printf(cu, "\t\tSKL_TKN_U32_DIR_PIN_COUNT \"%u\"\n", dpc);
	custr_append_printf(cu, "\t\tSKL_TKN_U32_FMT_CH \"%u\"\n",
	    fmt->channels);
	custr_append_printf(cu, "\t\tSKL_TKN_U32_FMT_FREQ \"%u\"\n",
	    fmt->freq);
	custr_append_printf(cu, "\t\tSKL_TKN_U32_FMT_BIT_DEPTH \"%u\"\n",
	    fmt->bit_depth);
	custr_append_printf(cu, "\t\tSKL_TKN_U32_FMT_SAMPLE_SIZE \"%u\"\n",
	    fmt->valid_bit_depth);
	custr_append_printf(cu, "\t\tSKL_TKN_U32_FMT_INTERLEAVE \"%u\"\n",
	    fmt->interleaving_style);
	custr_append_printf(cu, "\t\tSKL_TKN_U32_FMT_SAMPLE_TYPE \"%u\"\n",
	    fmt->sample_type);
	custr_append_printf(cu, "\t\tSKL_TKN_U32_FMT_CH_MAP \"0x%x\"\n",
	    fmt->ch_map);
	custr_append_printf(cu, "\t\tSKL_TKN_U32_FMT_CH_CONFIG \"0x%x\"\n",
	    fmt->ch_cfg);
}

static void
emit_pin(struct skl_dfw_module_pin *pin, custr_t *cu, unsigned dir, unsigned idx)
{
	unsigned dpc = dir | (idx << 4);

	custr_append_printf(cu, "\t\tSKL_TKN_U32_DIR_PIN_COUNT \"%u\"\n", dpc);
	custr_append_printf(cu, "\t\tSKL_TKN_U32_PIN_MOD_ID \"%u\"\n",
	    pin->module_id);
	custr_append_printf(cu, "\t\tSKL_TKN_U32_PIN_INST_ID \"%u\"\n",
	    pin->instance_id);
}

static void
add_data(char *name)
{
	custr_append_printf(s_data, "SectionData.\"%s num_desc\" {\n", name);
	custr_append_printf(s_data, "\ttuples \"%s num_desc\"\n", name);
	custr_append_printf(s_data, "}\n");
	custr_append_printf(s_data, "\n");

	custr_append_printf(s_data, "SectionData.\"%s_size_desc\" {\n", name);
	custr_append_printf(s_data, "\ttuples \"%s_size_desc\"\n", name);
	custr_append_printf(s_data, "}\n");
	custr_append_printf(s_data, "\n");

	custr_append_printf(s_data, "SectionData.\"%s\" {\n", name);
	custr_append_printf(s_data, "\ttuples \"%s\"\n", name);
	custr_append_printf(s_data, "}\n");
	custr_append_printf(s_data, "\n");
}

static void
print_mixer(struct snd_soc_tplg_mixer_control *m)
{
	custr_append_printf(s_mixers, "SectionControlMixer.\"%s\" {\n", m->hdr.name);
	custr_append_printf(s_mixers, "\tindex\"1\"\n");
	custr_append_printf(s_mixers, "\tinvert \"%s\"\n", m->invert ? "true" : "false");
	custr_append_printf(s_mixers, "\tmax \"%u\"\n", m->max);
	custr_append_printf(s_mixers, "\tmin\"%u\"\n", m->min);
	custr_append_printf(s_mixers, "\tno_pm \"true\"\n");

	for (unsigned i = 0; i < m->num_channels; i++) {
		const char *chnm = NULL;

		switch (m->channel[i].id) {
		case 3:
			chnm = "fl";
			break;
		case 4:
			chnm = "fr";
			break;
		default:
			fprintf(stderr, "UNKNOWN CHANNEL NUMBER: %u\n", m->channel[i].id);
			exit(1);
			break;
		}

		custr_append_printf(s_mixers, "\tchannel.\"%s\" {\n", chnm);
		custr_append_printf(s_mixers, "\t\treg \"%d\"\n", (int)m->channel[i].reg);
		custr_append_printf(s_mixers, "\t\tshift \"%u\"\n", m->channel[i].shift);
		custr_append_printf(s_mixers, "\t}\n");
	}

	custr_append_printf(s_mixers, "\tops.\"ctl\" {\n");
	custr_append_printf(s_mixers, "\t\tget \"%u\"\n", m->hdr.ops.get);
	custr_append_printf(s_mixers, "\t\tput \"%u\"\n", m->hdr.ops.put);
	custr_append_printf(s_mixers, "\t\tinfo \"%u\"\n", m->hdr.ops.info);
	custr_append_printf(s_mixers, "\t}\n");
	custr_append_printf(s_mixers, "}\n");

#if 0
	fprintf(stderr, "\t\t==== KCONTROL: MIXER ====\n");
	fprintf(stderr, "\t\tSIZE\t\t%u\n", m->size);
	fprintf(stderr, "\n");
	fprintf(stderr, "\t\tPRIV DATA SIZE\t%u\n", m->priv.size);
	fprintf(stderr, "\n");
#endif
}

static void
mixer_list(struct snd_soc_tplg_dapm_widget *w)
{
	void *pos = w;
	unsigned mixer_count = 0;

	pos += w->size + w->priv.size;

	for (unsigned i = 0; i < w->num_kcontrols; i++) {
		struct snd_soc_tplg_ctl_hdr *kc = pos;

#if 0
		fprintf(stderr, "\t\t==== KCONTROL %u ====\n", j);
		fprintf(stderr, "\t\tSIZE\t\t%u\n", kc->size);
		fprintf(stderr, "\t\tTYPE\t\t%u\n", kc->type);
#endif

		if (!lenchk(kc->name, SNDRV_CTL_ELEM_ID_NAME_MAXLEN)) {
			fprintf(stderr, "WARNING: invalid strings\n");
			return;
		}

#if 0
		fprintf(stderr, "\t\tNAME\t\t\"%s\"\n", kc->name);
#endif

		/*
		 * Check the "info" op type.  That seems to be
		 * the way that the kcontrol type is checked in
		 * the kernel.
		 */

#if 0
		fprintf(stderr, "\t\tOPS GET\t\t%u\n", kc->ops.get);
		fprintf(stderr, "\t\tOPS PUT\t\t%u\n", kc->ops.put);
		fprintf(stderr, "\t\tOPS INFO\t%s (%u)\n",
		    kcion, kc->ops.info);
#endif

		//fprintf(stderr, "\t\tTLV SIZE\t%u\n", kc->tlv.size);

		switch (kc->ops.info) {
		case SND_SOC_TPLG_DAPM_CTL_VOLSW: {
			struct snd_soc_tplg_mixer_control *m = pos;

			if (mixer_count++ == 0) {
				custr_append_printf(s_widgets, "\tmixer [\n");
			}
			custr_append_printf(s_widgets, "\t\t\"%s\"\n", kc->name);

			print_mixer(m);

			pos += m->size + m->priv.size;
			break;
		}

		case SND_SOC_TPLG_CTL_BYTES: {
			struct snd_soc_tplg_bytes_control *b = pos;

			/*
			 * XXX what to do about these?
			 */

			pos += b->size + b->priv.size;
			break;
		}

		default:
			fprintf(stderr, "WARNING: invalid kc info "
			    "op (%u)\n", kc->ops.info);
			return;
		}

		fprintf(stderr, "\n");


	}

	if (mixer_count > 0) {
		custr_append_printf(s_widgets, "\t]\n");
	}
}

static void
print_widgets(void *payload, unsigned nelems)
{
	void *pos = payload;
	unsigned i = 0;

	while (i < nelems) {
		struct snd_soc_tplg_dapm_widget *w = pos;

#if 0
		fprintf(stderr, "-- WIDGET %u ----------------------------\n",
		    i);
		fprintf(stderr, "\tSIZE\t\t%u\n", w->size);
#endif

		const char *wn = "<UNKNOWN>";
		if (w->id <= SND_SOC_TPLG_DAPM_LAST) {
			wn = widget_id_names[w->id];
		}

#if 0
		fprintf(stderr, "\tID\t\t%s (%u)\n", wn, w->id);
#endif

		/*
		 * Check string lengths.
		 */
		if (!lenchk(w->name, SNDRV_CTL_ELEM_ID_NAME_MAXLEN) ||
		    !lenchk(w->sname, SNDRV_CTL_ELEM_ID_NAME_MAXLEN)) {
			fprintf(stderr, "WARNING: invalid strings\n");
			return;
		}

		custr_append_printf(s_widgets, "SectionWidget.\"%s\" {\n",
		    w->name);
		custr_append_printf(s_widgets, "\tindex\"1\"\n");
		custr_append_printf(s_widgets, "\ttype\"%s\"\n", wn);
		custr_append_printf(s_widgets, "\tno_pm \"true\"\n");
		if (w->event_type != 0) {
			custr_append_printf(s_widgets, "\tevent_type \"%u\"\n", w->event_type);
		}
		if (w->event_flags != 0) {
			custr_append_printf(s_widgets, "\tevent_flags \"%u\"\n", w->event_flags);
		}
		if (w->subseq != 0) {
			custr_append_printf(s_widgets, "\tsubseq \"%u\"\n", w->subseq);
		}
		if (w->priv.size != 0) {
			custr_append_printf(s_widgets, "\tdata [\n");
			custr_append_printf(s_widgets, "\t\t\"%s num_desc\"\n", w->name);
			custr_append_printf(s_widgets, "\t\t\"%s_size_desc\"\n", w->name);
			custr_append_printf(s_widgets, "\t\t\"%s\"\n", w->name);
			custr_append_printf(s_widgets, "\t]\n");
		}
		if (w->num_kcontrols > 0) {
			/*
			 * We need to look for mixers now... Sigh.
			 */
			mixer_list(w);
		}

		custr_append_printf(s_widgets, "}\n");


		if (w->priv.size == 0)
			goto skip;

		struct skl_dfw_module *dfw = (struct skl_dfw_module *)w->priv.data;

		fprintf(stderr, "MODULE %s ID %u INSTID %u\n", w->name, dfw->module_id,
		    dfw->instance_id);

		if (!lenchk(dfw->uuid, SKL_UUID_STR_SZ)) {
			fprintf(stderr, "WARNING: invalid uuid\n");
			return;
		}

		add_data(w->name);

		/*
		 * Add "SectionVendorTuples" data to describe the private data
		 * struct.
		 */
		custr_append_printf(s_tuples, "SectionVendorTuples.\"%s num_desc\" {\n", w->name);
		custr_append_printf(s_tuples, "\ttokens \"skl_tokens\"\n");
		custr_append_printf(s_tuples, "\n");
		custr_append_printf(s_tuples, "\ttuples.\"byte.u8_num_blocks\" {\n");
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U8_NUM_BLOCKS \"1\"\n");
		custr_append_printf(s_tuples, "\t}\n");
		custr_append_printf(s_tuples, "}\n");
		custr_append_printf(s_tuples, "\n");

		custr_append_printf(s_tuples, "SectionVendorTuples.\"%s_size_desc\" {\n", w->name);
		custr_append_printf(s_tuples, "\ttokens \"skl_tokens\"\n");
		custr_append_printf(s_tuples, "\ttuples.\"byte.u8_block_type\"{\n");
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U8_BLOCK_TYPE \"0\"\n");
		custr_append_printf(s_tuples, "\t}\n");
		custr_append_printf(s_tuples, "\ttuples.\"short.u16_size_desc\"{\n");

		/*
		 * I'm not sure how this is derived, but based on observation of
		 * the file provided in "alsa-lib", I've determined that the
		 * formula is:
		 *
		 * 	BLOCK_SIZE = 220 + N * 96
		 *
		 * ... where N is the sum of input and output format/pin entries.
		 */

		unsigned bs = 220 + (dfw->max_in_queue + dfw->max_out_queue) * 96;
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U16_BLOCK_SIZE \"%u\"\n", bs);
		custr_append_printf(s_tuples, "\t}\n");
		custr_append_printf(s_tuples, "}\n");
		custr_append_printf(s_tuples, "\n");

		custr_append_printf(s_tuples, "SectionVendorTuples.\"%s\" {\n", w->name);
		custr_append_printf(s_tuples, "\ttokens \"skl_tokens\"\n");
		custr_append_printf(s_tuples, "\n");
		custr_append_printf(s_tuples, "\ttuples.\"uuid\" {\n");
		emit_uuid_tuple(dfw->uuid, s_tuples);
		custr_append_printf(s_tuples, "\t}\n");
		custr_append_printf(s_tuples, "\n");
		custr_append_printf(s_tuples, "\ttuples.\"byte.u8_data\" {\n");
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U8_IN_PIN_TYPE \"%u\"\n",
		    dfw->input_pin_type);
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U8_OUT_PIN_TYPE \"%u\"\n",
		    dfw->output_pin_type);
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U8_IN_QUEUE_COUNT \"%u\"\n",
		    dfw->max_in_queue);
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U8_OUT_QUEUE_COUNT \"%u\"\n",
		    dfw->max_out_queue);
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U8_DYN_IN_PIN \"%u\"\n",
		    dfw->is_dynamic_in_pin);
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U8_DYN_OUT_PIN \"%u\"\n",
		    dfw->is_dynamic_out_pin);
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U8_TIME_SLOT \"%u\"\n",
		    dfw->time_slot);
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U8_CORE_ID \"%u\"\n",
		    dfw->core_id);
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U8_MODULE_TYPE \"%u\"\n",
		    dfw->module_type);
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U8_CONN_TYPE \"%u\"\n",
		    dfw->conn_type);
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U8_HW_CONN_TYPE \"%u\"\n",
		    dfw->hw_conn_type);
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U8_DEV_TYPE \"%u\"\n",
		    dfw->dev_type);
		custr_append_printf(s_tuples, "\t}\n");
		custr_append_printf(s_tuples, "\n");
		custr_append_printf(s_tuples, "\ttuples.\"short.u16_data\" {\n");
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U16_MOD_INST_ID \"%u\"\n",
		    dfw->instance_id);
		custr_append_printf(s_tuples, "\t}\n");
		custr_append_printf(s_tuples, "\n");
		custr_append_printf(s_tuples, "\ttuples.\"word.u32_data\" {\n");
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U32_MAX_MCPS \"%u\"\n",
		    dfw->max_mcps);
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U32_MEM_PAGES \"%u\"\n",
		    dfw->mem_pages);
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U32_OBS \"%u\"\n",
		    dfw->obs);
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U32_IBS \"%u\"\n",
		    dfw->ibs);
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U32_VBUS_ID \"0x%x\"\n",
		    dfw->vbus_id);
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U32_PARAMS_FIXUP \"%u\"\n",
		    dfw->params_fixup);
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U32_CONVERTER \"%u\"\n",
		    dfw->converter);
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U32_PIPE_ID \"%u\"\n",
		    dfw->pipe.pipe_id);
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U32_PIPE_CONN_TYPE \"%u\"\n",
		    dfw->pipe.conn_type);
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U32_PIPE_PRIORITY \"%u\"\n",
		    dfw->pipe.pipe_priority);
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U32_PIPE_MEM_PGS \"%u\"\n",
		    dfw->pipe.memory_pages);
		custr_append_printf(s_tuples, "\t\tSKL_TKN_U32_CAPS_SIZE \"%u\"\n",
		    dfw->caps.caps_size);
		custr_append_printf(s_tuples, "\t}\n");
		custr_append_printf(s_tuples, "\n");

		/*
		 * Emit each input and output format data.
		 */
		for (unsigned q = 0; q < dfw->max_in_queue; q++) {
			/*
			 * IN FORMAT
			 */
			custr_append_printf(s_tuples, "\ttuples.\"word.in_fmt_%u\" {\n", q);
			emit_format(&dfw->in_fmt[q], s_tuples, 0, q);
			custr_append_printf(s_tuples, "\t}\n");
			custr_append_printf(s_tuples, "\n");
		}

		for (unsigned q = 0; q < dfw->max_out_queue; q++) {
			/*
			 * OUT FORMAT
			 */
			custr_append_printf(s_tuples, "\ttuples.\"word.out_fmt_%u\" {\n", q);
			emit_format(&dfw->out_fmt[q], s_tuples, 1, q);
			custr_append_printf(s_tuples, "\t}\n");
			custr_append_printf(s_tuples, "\n");
		}

		for (unsigned q = 0; q < dfw->max_in_queue; q++) {
			/*
			 * IN PIN
			 */
			custr_append_printf(s_tuples, "\ttuples.\"word.in_pin_%u\" {\n", q);
			emit_pin(&dfw->in_pin[q], s_tuples, 0, q);
			custr_append_printf(s_tuples, "\t}\n");
			custr_append_printf(s_tuples, "\n");
		}

		for (unsigned q = 0; q < dfw->max_out_queue; q++) {
			/*
			 * OUT PIN
			 */
			custr_append_printf(s_tuples, "\ttuples.\"word.out_pin_%u\" {\n", q);
			emit_pin(&dfw->out_pin[q], s_tuples, 1, q);
			custr_append_printf(s_tuples, "\t}\n");
			custr_append_printf(s_tuples, "\n");
		}

		custr_append_printf(s_tuples, "\n");
		custr_append_printf(s_tuples, "}\n");
		custr_append_printf(s_tuples, "\n");

skip:

#if 0
		fprintf(stderr, "\tNAME\t\t\"%s\"\n", w->name);
		fprintf(stderr, "\tSNAME\t\t\"%s\"\n", w->sname);

		fprintf(stderr, "\tREG\t\t0x%x\n", w->reg);
		fprintf(stderr, "\tSHIFT\t\t%u\n", w->shift);
		fprintf(stderr, "\tMASK\t\t0x%x\n", w->mask);
		fprintf(stderr, "\tSUBSEQ\t\t0x%x\n", w->subseq);
		fprintf(stderr, "\tINVERT\t\t%s\n", w->invert ? "YES" : "NO");
		fprintf(stderr, "\tIGNORE SUSPEND\t0x%x\n", w->ignore_suspend);
		fprintf(stderr, "\tEVENT FLAGS\t0x%x\n",
		    (unsigned)w->event_flags);
		fprintf(stderr, "\tEVENT TYPE\t0x%x\n",
		    (unsigned)w->event_flags);
		fprintf(stderr, "\t#KCONTROLS\t%u\n", w->num_kcontrols);

		fprintf(stderr, "\n");
		fprintf(stderr, "\tPRIV DATA SIZE\t%u\n", w->priv.size);
		fprintf(stderr, "\n");

		if (w->priv.size != 0) {
			if (w->priv.size != sizeof (struct skl_dfw_module)) {
				fprintf(stderr, "WARNING: invalid private "
				    "data size\n");
				return;
			}

			struct skl_dfw_module *dfw =
			    (struct skl_dfw_module *)w->priv.data;

			if (!lenchk(dfw->uuid, SKL_UUID_STR_SZ)) {
				fprintf(stderr, "WARNING: invalid uuid\n");
				return;
			}

			fprintf(stderr, "\tUUID\t\t\"%s\"\n", dfw->uuid);

			fprintf(stderr, "\n");
		}
#endif

		pos += w->size + w->priv.size;

		unsigned j = 0;

		while (j < w->num_kcontrols) {
			struct snd_soc_tplg_ctl_hdr *kc = pos;

			fprintf(stderr, "\t\t==== KCONTROL %u ====\n", j);
			fprintf(stderr, "\t\tSIZE\t\t%u\n", kc->size);
			fprintf(stderr, "\t\tTYPE\t\t%u\n", kc->type);

			if (!lenchk(kc->name, SNDRV_CTL_ELEM_ID_NAME_MAXLEN)) {
				fprintf(stderr, "WARNING: invalid strings\n");
				return;
			}
			fprintf(stderr, "\t\tNAME\t\t\"%s\"\n", kc->name);

			/*
			 * Check the "info" op type.  That seems to be
			 * the way that the kcontrol type is checked in
			 * the kernel.
			 */
			const char *kcion = NULL;
			if (kc->ops.info < 256) {
				kcion = kc_info_op_names[kc->ops.info];
			}

			if (kcion == NULL) {
				fprintf(stderr, "WARNING: invalid kc info "
				    "op\n");
				return;
			}

			fprintf(stderr, "\t\tOPS GET\t\t%u\n", kc->ops.get);
			fprintf(stderr, "\t\tOPS PUT\t\t%u\n", kc->ops.put);
			fprintf(stderr, "\t\tOPS INFO\t%s (%u)\n",
			    kcion, kc->ops.info);

			//fprintf(stderr, "\t\tTLV SIZE\t%u\n", kc->tlv.size);

			switch (kc->ops.info) {
			case SND_SOC_TPLG_DAPM_CTL_VOLSW: {
				struct snd_soc_tplg_mixer_control *m = pos;

				//print_mixer(m);

				pos += m->size + m->priv.size;
				break;
			}

			case SND_SOC_TPLG_CTL_BYTES: {
				struct snd_soc_tplg_bytes_control *b = pos;

				pos += b->size + b->priv.size;
				break;
			}

			default:
				fprintf(stderr, "WARNING: invalid kc info "
				    "op (%s)\n", kcion);
				return;
			}

			fprintf(stderr, "\n");


			j++;
		}

		i++;

		fprintf(stderr, "\n");
	}
}

int
main(int argc, char *argv[])
{
	int fd;
	struct stat st;

	if (argc != 2) {
		errx(1, "usage: %s <dfw_sst.bin>", argv[0]);
	}

	if (custr_alloc(&s_widgets) != 0 ||
	    custr_alloc(&s_data) != 0 ||
	    custr_alloc(&s_mixers) != 0 ||
	    custr_alloc(&s_tuples) != 0 ||
	    custr_alloc(&s_graph) != 0) {
		err(1, "custr_alloc");
	}

	if ((fd = open(argv[1], O_RDONLY)) < 0) {
		err(1, "open \"%s\"", argv[1]);
	}

	if (fstat(fd, &st) != 0) {
		err(1, "stat \"%s\"", argv[1]);
	}
	fprintf(stderr, "size: %lu\n", st.st_size);

	fprintf(stderr, "expected header size: %u\n",
	    sizeof (struct snd_soc_tplg_hdr));

	if ((data = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED,
	    fd, 0)) == MAP_FAILED) {
		err(1, "mmap");
	}

	fprintf(stderr, "\n");

	void *pos = data;
	for (;;) {
		struct snd_soc_tplg_hdr *hdr = pos;
		void *payload = pos + hdr->size;

		if (pos + sizeof (*hdr) >= data + st.st_size)
			break;

		if (hdr->magic != SND_SOC_TPLG_MAGIC) {
			fprintf(stderr, "WARNING: unexpected magic\n");
		}

		print_header(hdr);

		switch (hdr->type) {
		case SND_SOC_TPLG_TYPE_MANIFEST:
			print_manifest(payload);
			break;
		case SND_SOC_TPLG_TYPE_DAPM_WIDGET:
			print_widgets(payload, hdr->count);
			break;
		case SND_SOC_TPLG_TYPE_DAPM_GRAPH:
			print_graph(hdr, payload, hdr->count);
			break;
		default:
			break;
		}

		pos += hdr->size + hdr->payload_size;

		fprintf(stderr, "\n");
	}

	if (pos != data + st.st_size) {
		fprintf(stderr, "WARNING: did not reach real EOF\n");
	}

	fprintf(stderr, "\n");

	fflush(stderr);

	fprintf(stdout, "%s\n", custr_cstr(s_tuples));
	fprintf(stdout, "%s\n", custr_cstr(s_data));
	fprintf(stdout, "%s\n", custr_cstr(s_mixers));
	fprintf(stdout, "%s\n", custr_cstr(s_widgets));
	fprintf(stdout, "%s\n", custr_cstr(s_graph));

	fflush(stdout);

	return (0);
}
