

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <err.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>


#include "local_asoc.h"
#include "local_skl-tplg-interface.h"


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
	[SND_SOC_TPLG_DAPM_INPUT] = "INPUT",
	[SND_SOC_TPLG_DAPM_OUTPUT] = "OUTPUT",
	[SND_SOC_TPLG_DAPM_MUX] = "MUX",
	[SND_SOC_TPLG_DAPM_MIXER] = "mixer",
	[SND_SOC_TPLG_DAPM_PGA] = "pga",
	[SND_SOC_TPLG_DAPM_OUT_DRV] = "OUT_DRV",
	[SND_SOC_TPLG_DAPM_ADC] = "ADC",
	[SND_SOC_TPLG_DAPM_DAC] = "DAC",
	[SND_SOC_TPLG_DAPM_SWITCH] = "SWITCH",
	[SND_SOC_TPLG_DAPM_PRE] = "PRE",
	[SND_SOC_TPLG_DAPM_POST] = "POST",
	[SND_SOC_TPLG_DAPM_AIF_IN] = "aif_in",
	[SND_SOC_TPLG_DAPM_AIF_OUT] = "aif_out",
	[SND_SOC_TPLG_DAPM_DAI_IN] = "DAI_IN",
	[SND_SOC_TPLG_DAPM_DAI_OUT] = "DAI_OUT",
	[SND_SOC_TPLG_DAPM_DAI_LINK] = "DAI_LINK",
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



#define	PATH	"dfw_sst.bin"

void *data = NULL;

static void
print_header(struct snd_soc_tplg_hdr *hdr)
{
	fprintf(stdout, "HEADER %p (offset %u)\n", (void *)hdr,
	    (unsigned)((void *)hdr - data));

	char mag[4];
	memcpy(mag, &hdr->magic, 4);
	fprintf(stdout, "\tMAGIC\t\t%c%c%c%c\n", mag[3],
	    mag[2], mag[1], mag[0]);

	fprintf(stdout, "\tABI\t\t%u\n", hdr->abi);

	fprintf(stdout, "\tVERSION\t\t%u\n", hdr->version);

	const char *tn = "<UNKNOWN>";
	if (hdr->type <= SND_SOC_TPLG_TYPE_MAX)
		tn = type_names[hdr->type];
	fprintf(stdout, "\tTYPE\t\t%s (%u)\n", tn, hdr->type);

	fprintf(stdout, "\tHEADER SIZE\t%u\n", hdr->size);

	fprintf(stdout, "\tVENDOR TYPE\t%u\n", hdr->vendor_type);

	fprintf(stdout, "\tPAYLOAD SIZE\t%u\n", hdr->payload_size);

	fprintf(stdout, "\tINDEX\t\t%u\n", hdr->index);

	fprintf(stdout, "\tCOUNT\t\t%u\n", hdr->count);
}

static void
print_manifest(struct snd_soc_tplg_manifest *manifest)
{
	fprintf(stdout, "-- MANIFEST ----------------------------\n");
	fprintf(stdout, "\tSIZE\t\t%u\n", manifest->size);
	fprintf(stdout, "\t#CONTROL\t%u\n", manifest->control_elems);
	fprintf(stdout, "\t#WIDGET\t\t%u\n", manifest->widget_elems);
	fprintf(stdout, "\t#GRAPH\t\t%u\n", manifest->graph_elems);
	fprintf(stdout, "\t#DAI\t\t%u\n", manifest->dai_elems);
	fprintf(stdout, "\t#DAI LINK\t%u\n", manifest->dai_link_elems);
	fprintf(stdout, "\n");
	fprintf(stdout, "\tPRIV DATA SIZE\t%u\n", manifest->priv.size);
}

static int
lenchk(char *x, size_t maxlen)
{
	return (strnlen(x, maxlen) < maxlen);
}

static void
print_mixer(struct snd_soc_tplg_mixer_control *m)
{
	fprintf(stdout, "\t\t==== KCONTROL: MIXER ====\n");
	fprintf(stdout, "\t\tSIZE\t\t%u\n", m->size);
	fprintf(stdout, "\n");
	fprintf(stdout, "\t\tPRIV DATA SIZE\t%u\n", m->priv.size);
	fprintf(stdout, "\n");
}

static void
print_graph(struct snd_soc_tplg_hdr *hdr, void *payload, unsigned nelems)
{
	void *pos = payload;
	unsigned i = 0;

	fprintf(stdout, "SectionGraph.\"Pipeline %u Graph\" {\n",
	    hdr->index + 1);
	fprintf(stdout, "        index\"%u\"\n", hdr->index + 1);
	fprintf(stdout, "        lines [\n");

	while (i < nelems) {
		struct snd_soc_tplg_dapm_graph_elem *g = pos;

#if 0
		fprintf(stdout, "-- GRAPH ELEMENT %u ----------------------\n",
		    i);
#endif

		if (!lenchk(g->sink, SNDRV_CTL_ELEM_ID_NAME_MAXLEN) ||
		    !lenchk(g->control, SNDRV_CTL_ELEM_ID_NAME_MAXLEN) ||
		    !lenchk(g->source, SNDRV_CTL_ELEM_ID_NAME_MAXLEN)) {
			fprintf(stdout, "WARNING: invalid string\n");
			return;
		}

		fprintf(stdout, "                \"%s, %s, %s\"\n",
		    g->sink, g->control, g->source);

#if 0
		fprintf(stdout, "\tSINK\t\t\"%s\"\n", g->sink);
		fprintf(stdout, "\tCONTROL\t\t\"%s\"\n", g->control);
		fprintf(stdout, "\tSOURCE\t\t\"%s\"\n", g->source);
#endif

		pos += sizeof (*g);
		i++;
	}

	fprintf(stdout, "        ]\n");
	fprintf(stdout, "}\n");
}

static void
print_widgets(void *payload, unsigned nelems)
{
	void *pos = payload;
	unsigned i = 0;

	while (i < nelems) {
		struct snd_soc_tplg_dapm_widget *w = pos;

		fprintf(stdout, "-- WIDGET %u ----------------------------\n",
		    i);
		fprintf(stdout, "\tSIZE\t\t%u\n", w->size);

		const char *wn = "<UNKNOWN>";
		if (w->id <= SND_SOC_TPLG_DAPM_LAST) {
			wn = widget_id_names[w->id];
		}

		fprintf(stdout, "\tID\t\t%s (%u)\n", wn, w->id);

		/*
		 * Check string lengths.
		 */
		if (!lenchk(w->name, SNDRV_CTL_ELEM_ID_NAME_MAXLEN) ||
		    !lenchk(w->sname, SNDRV_CTL_ELEM_ID_NAME_MAXLEN)) {
			fprintf(stdout, "WARNING: invalid strings\n");
			return;
		}

		fprintf(stdout, "SectionWidget.\"%s\" {\n", w->name);
		fprintf(stdout, "        index\"1\"\n");
		fprintf(stdout, "        type\"%s\"\n", wn);
		fprintf(stdout, "}\n");

		fprintf(stdout, "\tNAME\t\t\"%s\"\n", w->name);
		fprintf(stdout, "\tSNAME\t\t\"%s\"\n", w->sname);

		fprintf(stdout, "\tREG\t\t0x%x\n", w->reg);
		fprintf(stdout, "\tSHIFT\t\t%u\n", w->shift);
		fprintf(stdout, "\tMASK\t\t0x%x\n", w->mask);
		fprintf(stdout, "\tSUBSEQ\t\t0x%x\n", w->subseq);
		fprintf(stdout, "\tINVERT\t\t%s\n", w->invert ? "YES" : "NO");
		fprintf(stdout, "\tIGNORE SUSPEND\t0x%x\n", w->ignore_suspend);
		fprintf(stdout, "\tEVENT FLAGS\t0x%x\n",
		    (unsigned)w->event_flags);
		fprintf(stdout, "\tEVENT TYPE\t0x%x\n",
		    (unsigned)w->event_flags);
		fprintf(stdout, "\t#KCONTROLS\t%u\n", w->num_kcontrols);

		fprintf(stdout, "\n");
		fprintf(stdout, "\tPRIV DATA SIZE\t%u\n", w->priv.size);
		fprintf(stdout, "\n");

		if (w->priv.size != 0) {
			if (w->priv.size != sizeof (struct skl_dfw_module)) {
				fprintf(stdout, "WARNING: invalid private "
				    "data size\n");
				return;
			}

			struct skl_dfw_module *dfw =
			    (struct skl_dfw_module *)w->priv.data;

			if (!lenchk(dfw->uuid, SKL_UUID_STR_SZ)) {
				fprintf(stdout, "WARNING: invalid uuid\n");
				return;
			}

			fprintf(stdout, "\tUUID\t\t\"%s\"\n", dfw->uuid);

			fprintf(stdout, "\n");
		}

		pos += w->size + w->priv.size;

		unsigned j = 0;

		while (j < w->num_kcontrols) {
			struct snd_soc_tplg_ctl_hdr *kc = pos;

			fprintf(stdout, "\t\t==== KCONTROL %u ====\n", j);
			fprintf(stdout, "\t\tSIZE\t\t%u\n", kc->size);
			fprintf(stdout, "\t\tTYPE\t\t%u\n", kc->type);

			if (!lenchk(kc->name, SNDRV_CTL_ELEM_ID_NAME_MAXLEN)) {
				fprintf(stdout, "WARNING: invalid strings\n");
				return;
			}
			fprintf(stdout, "\t\tNAME\t\t\"%s\"\n", kc->name);

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
				fprintf(stdout, "WARNING: invalid kc info "
				    "op\n");
				return;
			}

			fprintf(stdout, "\t\tOPS GET\t\t%u\n", kc->ops.get);
			fprintf(stdout, "\t\tOPS PUT\t\t%u\n", kc->ops.put);
			fprintf(stdout, "\t\tOPS INFO\t%s (%u)\n",
			    kcion, kc->ops.info);

			//fprintf(stdout, "\t\tTLV SIZE\t%u\n", kc->tlv.size);

			switch (kc->ops.info) {
			case SND_SOC_TPLG_DAPM_CTL_VOLSW: {
				struct snd_soc_tplg_mixer_control *m = pos;

				print_mixer(m);

				pos += m->size + m->priv.size;
				break;
			}

			case SND_SOC_TPLG_CTL_BYTES: {
				struct snd_soc_tplg_bytes_control *b = pos;

				pos += b->size + b->priv.size;
				break;
			}

			default:
				fprintf(stdout, "WARNING: invalid kc info "
				    "op (%s)\n", kcion);
				return;
			}

			fprintf(stdout, "\n");


			j++;
		}

		i++;

		fprintf(stdout, "\n");
	}
}

int
main(int argc, char *argv[])
{
	int fd;
	struct stat st;

	if ((fd = open(PATH, O_RDONLY)) < 0) {
		err(1, "open \"%s\"", PATH);
	}

	if (fstat(fd, &st) != 0) {
		err(1, "stat \"%s\"", PATH);
	}
	fprintf(stdout, "size: %lu\n", st.st_size);

	fprintf(stdout, "expected header size: %u\n",
	    sizeof (struct snd_soc_tplg_hdr));

	if ((data = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED,
	    fd, 0)) == MAP_FAILED) {
		err(1, "mmap");
	}

	fprintf(stdout, "\n");

	void *pos = data;
	for (;;) {
		struct snd_soc_tplg_hdr *hdr = pos;
		void *payload = pos + hdr->size;

		if (pos + sizeof (*hdr) >= data + st.st_size)
			break;

		if (hdr->magic != SND_SOC_TPLG_MAGIC) {
			fprintf(stdout, "WARNING: unexpected magic\n");
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

		fprintf(stdout, "\n");
	}

	if (pos != data + st.st_size) {
		fprintf(stdout, "WARNING: did not reach real EOF\n");
	}

	fprintf(stdout, "\n");

	return (0);
}
