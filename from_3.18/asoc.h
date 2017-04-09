/*
 * uapi/sound/asoc.h -- ALSA SoC Firmware Controls and DAPM
 *
 * Copyright (C) 2012 Texas Instruments Inc.
 * Copyright (C) 2015 Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Simple file API to load FW that includes mixers, coefficients, DAPM graphs,
 * algorithms, equalisers, DAIs, widgets etc.
*/

#ifndef __LINUX_UAPI_SND_ASOC_H
#define __LINUX_UAPI_SND_ASOC_H

/*
 * XXX from other places...
 */
#define SNDRV_CTL_ELEM_ID_NAME_MAXLEN   44


/*
 * Maximum number of channels topology kcontrol can represent.
 */
#define SND_SOC_TPLG_MAX_CHAN		8

/*
 * Maximum number of PCM formats capability
 */
#define SND_SOC_TPLG_MAX_FORMATS	16

/*
 * Maximum number of PCM stream configs
 */
#define SND_SOC_TPLG_STREAM_CONFIG_MAX  8

/* individual kcontrol info types - can be mixed with other types */
#define SND_SOC_TPLG_CTL_VOLSW		1
#define SND_SOC_TPLG_CTL_VOLSW_SX	2
#define SND_SOC_TPLG_CTL_VOLSW_XR_SX	3
#define SND_SOC_TPLG_CTL_ENUM		4
#define SND_SOC_TPLG_CTL_BYTES		5
#define SND_SOC_TPLG_CTL_ENUM_VALUE	6
#define SND_SOC_TPLG_CTL_RANGE		7
#define SND_SOC_TPLG_CTL_STROBE		8


/* individual widget kcontrol info types - can be mixed with other types */
#define SND_SOC_TPLG_DAPM_CTL_VOLSW		64
#define SND_SOC_TPLG_DAPM_CTL_ENUM_DOUBLE	65
#define SND_SOC_TPLG_DAPM_CTL_ENUM_VIRT		66
#define SND_SOC_TPLG_DAPM_CTL_ENUM_VALUE	67
#define SND_SOC_TPLG_DAPM_CTL_PIN		68

/* DAPM widget types - add new items to the end */
#define SND_SOC_TPLG_DAPM_INPUT		0
#define SND_SOC_TPLG_DAPM_OUTPUT	1
#define SND_SOC_TPLG_DAPM_MUX		2
#define SND_SOC_TPLG_DAPM_MIXER		3
#define SND_SOC_TPLG_DAPM_PGA		4
#define SND_SOC_TPLG_DAPM_OUT_DRV	5
#define SND_SOC_TPLG_DAPM_ADC		6
#define SND_SOC_TPLG_DAPM_DAC		7
#define SND_SOC_TPLG_DAPM_SWITCH	8
#define SND_SOC_TPLG_DAPM_PRE		9
#define SND_SOC_TPLG_DAPM_POST		10
#define SND_SOC_TPLG_DAPM_AIF_IN	11
#define SND_SOC_TPLG_DAPM_AIF_OUT	12
#define SND_SOC_TPLG_DAPM_DAI_IN	13
#define SND_SOC_TPLG_DAPM_DAI_OUT	14
#define SND_SOC_TPLG_DAPM_DAI_LINK	15
#define SND_SOC_TPLG_DAPM_LAST		SND_SOC_TPLG_DAPM_DAI_LINK

/* Header magic number and string sizes */
#define SND_SOC_TPLG_MAGIC		0x41536F43 /* ASoC */

/* string sizes */
#define SND_SOC_TPLG_NUM_TEXTS		16

/* ABI version */
#define SND_SOC_TPLG_ABI_VERSION	0x4

/* Max size of TLV data */
#define SND_SOC_TPLG_TLV_SIZE		32

/*
 * File and Block header data types.
 * Add new generic and vendor types to end of list.
 * Generic types are handled by the core whilst vendors types are passed
 * to the component drivers for handling.
 */
#define SND_SOC_TPLG_TYPE_MIXER		1
#define SND_SOC_TPLG_TYPE_BYTES		2
#define SND_SOC_TPLG_TYPE_ENUM		3
#define SND_SOC_TPLG_TYPE_DAPM_GRAPH	4
#define SND_SOC_TPLG_TYPE_DAPM_WIDGET	5
#define SND_SOC_TPLG_TYPE_DAI_LINK	6
#define SND_SOC_TPLG_TYPE_PCM		7
#define SND_SOC_TPLG_TYPE_MANIFEST	8
#define SND_SOC_TPLG_TYPE_CODEC_LINK	9
#define SND_SOC_TPLG_TYPE_BACKEND_LINK	10
#define SND_SOC_TPLG_TYPE_PDATA		11
#define SND_SOC_TPLG_TYPE_MAX	SND_SOC_TPLG_TYPE_PDATA

/* vendor block IDs - please add new vendor types to end */
#define SND_SOC_TPLG_TYPE_VENDOR_FW	1000
#define SND_SOC_TPLG_TYPE_VENDOR_CONFIG	1001
#define SND_SOC_TPLG_TYPE_VENDOR_COEFF	1002
#define SND_SOC_TPLG_TYPEVENDOR_CODEC	1003

#define SND_SOC_TPLG_STREAM_PLAYBACK	0
#define SND_SOC_TPLG_STREAM_CAPTURE	1

/*
 * Block Header.
 * This header precedes all object and object arrays below.
 */
struct snd_soc_tplg_hdr {
	uint32_t magic;		/* magic number */
	uint32_t abi;		/* ABI version */
	uint32_t version;		/* optional vendor specific version details */
	uint32_t type;		/* SND_SOC_TPLG_TYPE_ */
	uint32_t size;		/* size of this structure */
	uint32_t vendor_type;	/* optional vendor specific type info */
	uint32_t payload_size;	/* data bytes, excluding this header */
	uint32_t index;		/* identifier for block */
	uint32_t count;		/* number of elements in block */
} __attribute__((packed));

/*
 * Private data.
 * All topology objects may have private data that can be used by the driver or
 * firmware. Core will ignore this data.
 */
struct snd_soc_tplg_private {
	uint32_t size;	/* in bytes of private data */
	char data[0];
} __attribute__((packed));

/*
 * Kcontrol TLV data.
 */
struct snd_soc_tplg_tlv_dbscale {
	uint32_t min;
	uint32_t step;
	uint32_t mute;
} __attribute__((packed));

struct snd_soc_tplg_ctl_tlv {
	uint32_t size;	/* in bytes of this structure */
	uint32_t type;	/* SNDRV_CTL_TLVT_*, type of TLV */
	union {
		uint32_t data[SND_SOC_TPLG_TLV_SIZE];
		struct snd_soc_tplg_tlv_dbscale scale;
	};
} __attribute__((packed));

/*
 * Kcontrol channel data
 */
struct snd_soc_tplg_channel {
	uint32_t size;	/* in bytes of this structure */
	uint32_t reg;
	uint32_t shift;
	uint32_t id;	/* ID maps to Left, Right, LFE etc */
} __attribute__((packed));

/*
 * Genericl Operations IDs, for binding Kcontrol or Bytes ext ops
 * Kcontrol ops need get/put/info.
 * Bytes ext ops need get/put.
 */
struct snd_soc_tplg_io_ops {
	uint32_t get;
	uint32_t put;
	uint32_t info;
} __attribute__((packed));

/*
 * kcontrol header
 */
struct snd_soc_tplg_ctl_hdr {
	uint32_t size;	/* in bytes of this structure */
	uint32_t type;
	char name[SNDRV_CTL_ELEM_ID_NAME_MAXLEN];
	uint32_t access;
	struct snd_soc_tplg_io_ops ops;
	struct snd_soc_tplg_ctl_tlv tlv;
} __attribute__((packed));

/*
 * Stream Capabilities
 */
struct snd_soc_tplg_stream_caps {
	uint32_t size;		/* in bytes of this structure */
	char name[SNDRV_CTL_ELEM_ID_NAME_MAXLEN];
	uint64_t formats;	/* supported formats SNDRV_PCM_FMTBIT_* */
	uint32_t rates;		/* supported rates SNDRV_PCM_RATE_* */
	uint32_t rate_min;	/* min rate */
	uint32_t rate_max;	/* max rate */
	uint32_t channels_min;	/* min channels */
	uint32_t channels_max;	/* max channels */
	uint32_t periods_min;	/* min number of periods */
	uint32_t periods_max;	/* max number of periods */
	uint32_t period_size_min;	/* min period size bytes */
	uint32_t period_size_max;	/* max period size bytes */
	uint32_t buffer_size_min;	/* min buffer size bytes */
	uint32_t buffer_size_max;	/* max buffer size bytes */
} __attribute__((packed));

/*
 * FE or BE Stream configuration supported by SW/FW
 */
struct snd_soc_tplg_stream {
	uint32_t size;		/* in bytes of this structure */
	char name[SNDRV_CTL_ELEM_ID_NAME_MAXLEN]; /* Name of the stream */
	uint64_t format;		/* SNDRV_PCM_FMTBIT_* */
	uint32_t rate;		/* SNDRV_PCM_RATE_* */
	uint32_t period_bytes;	/* size of period in bytes */
	uint32_t buffer_bytes;	/* size of buffer in bytes */
	uint32_t channels;	/* channels */
} __attribute__((packed));

/*
 * Manifest. List totals for each payload type. Not used in parsing, but will
 * be passed to the component driver before any other objects in order for any
 * global component resource allocations.
 *
 * File block representation for manifest :-
 * +-----------------------------------+----+
 * | struct snd_soc_tplg_hdr           |  1 |
 * +-----------------------------------+----+
 * | struct snd_soc_tplg_manifest      |  1 |
 * +-----------------------------------+----+
 */
struct snd_soc_tplg_manifest {
	uint32_t size;		/* in bytes of this structure */
	uint32_t control_elems;	/* number of control elements */
	uint32_t widget_elems;	/* number of widget elements */
	uint32_t graph_elems;	/* number of graph elements */
	uint32_t dai_elems;	/* number of DAI elements */
	uint32_t dai_link_elems;	/* number of DAI link elements */
	struct snd_soc_tplg_private priv;
} __attribute__((packed));

/*
 * Mixer kcontrol.
 *
 * File block representation for mixer kcontrol :-
 * +-----------------------------------+----+
 * | struct snd_soc_tplg_hdr           |  1 |
 * +-----------------------------------+----+
 * | struct snd_soc_tplg_mixer_control |  N |
 * +-----------------------------------+----+
 */
struct snd_soc_tplg_mixer_control {
	struct snd_soc_tplg_ctl_hdr hdr;
	uint32_t size;	/* in bytes of this structure */
	uint32_t min;
	uint32_t max;
	uint32_t platform_max;
	uint32_t invert;
	uint32_t num_channels;
	struct snd_soc_tplg_channel channel[SND_SOC_TPLG_MAX_CHAN];
	struct snd_soc_tplg_private priv;
} __attribute__((packed));

/*
 * Enumerated kcontrol
 *
 * File block representation for enum kcontrol :-
 * +-----------------------------------+----+
 * | struct snd_soc_tplg_hdr           |  1 |
 * +-----------------------------------+----+
 * | struct snd_soc_tplg_enum_control  |  N |
 * +-----------------------------------+----+
 */
struct snd_soc_tplg_enum_control {
	struct snd_soc_tplg_ctl_hdr hdr;
	uint32_t size;	/* in bytes of this structure */
	uint32_t num_channels;
	struct snd_soc_tplg_channel channel[SND_SOC_TPLG_MAX_CHAN];
	uint32_t items;
	uint32_t mask;
	uint32_t count;
	char texts[SND_SOC_TPLG_NUM_TEXTS][SNDRV_CTL_ELEM_ID_NAME_MAXLEN];
	uint32_t values[SND_SOC_TPLG_NUM_TEXTS * SNDRV_CTL_ELEM_ID_NAME_MAXLEN / 4];
	struct snd_soc_tplg_private priv;
} __attribute__((packed));

/*
 * Bytes kcontrol
 *
 * File block representation for bytes kcontrol :-
 * +-----------------------------------+----+
 * | struct snd_soc_tplg_hdr           |  1 |
 * +-----------------------------------+----+
 * | struct snd_soc_tplg_bytes_control |  N |
 * +-----------------------------------+----+
 */
struct snd_soc_tplg_bytes_control {
	struct snd_soc_tplg_ctl_hdr hdr;
	uint32_t size;	/* in bytes of this structure */
	uint32_t max;
	uint32_t mask;
	uint32_t base;
	uint32_t num_regs;
	struct snd_soc_tplg_io_ops ext_ops;
	struct snd_soc_tplg_private priv;
} __attribute__((packed));

/*
 * DAPM Graph Element
 *
 * File block representation for DAPM graph elements :-
 * +-------------------------------------+----+
 * | struct snd_soc_tplg_hdr             |  1 |
 * +-------------------------------------+----+
 * | struct snd_soc_tplg_dapm_graph_elem |  N |
 * +-------------------------------------+----+
 */
struct snd_soc_tplg_dapm_graph_elem {
	char sink[SNDRV_CTL_ELEM_ID_NAME_MAXLEN];
	char control[SNDRV_CTL_ELEM_ID_NAME_MAXLEN];
	char source[SNDRV_CTL_ELEM_ID_NAME_MAXLEN];
} __attribute__((packed));

/*
 * DAPM Widget.
 *
 * File block representation for DAPM widget :-
 * +-------------------------------------+-----+
 * | struct snd_soc_tplg_hdr             |  1  |
 * +-------------------------------------+-----+
 * | struct snd_soc_tplg_dapm_widget     |  N  |
 * +-------------------------------------+-----+
 * |   struct snd_soc_tplg_enum_control  | 0|1 |
 * |   struct snd_soc_tplg_mixer_control | 0|N |
 * +-------------------------------------+-----+
 *
 * Optional enum or mixer control can be appended to the end of each widget
 * in the block.
 */
struct snd_soc_tplg_dapm_widget {
	uint32_t size;		/* in bytes of this structure */
	uint32_t id;		/* SND_SOC_DAPM_CTL */
	char name[SNDRV_CTL_ELEM_ID_NAME_MAXLEN];
	char sname[SNDRV_CTL_ELEM_ID_NAME_MAXLEN];

	uint32_t reg;		/* negative reg = no direct dapm */
	uint32_t shift;		/* bits to shift */
	uint32_t mask;		/* non-shifted mask */
	uint32_t subseq;		/* sort within widget type */
	uint32_t invert;		/* invert the power bit */
	uint32_t ignore_suspend;	/* kept enabled over suspend */
	uint16_t event_flags;
	uint16_t event_type;
	uint32_t num_kcontrols;
	struct snd_soc_tplg_private priv;
	/*
	 * kcontrols that relate to this widget
	 * follow here after widget private data
	 */
} __attribute__((packed));


/*
 * Describes SW/FW specific features of PCM (FE DAI & DAI link).
 *
 * File block representation for PCM :-
 * +-----------------------------------+-----+
 * | struct snd_soc_tplg_hdr           |  1  |
 * +-----------------------------------+-----+
 * | struct snd_soc_tplg_pcm           |  N  |
 * +-----------------------------------+-----+
 */
struct snd_soc_tplg_pcm {
	uint32_t size;		/* in bytes of this structure */
	char pcm_name[SNDRV_CTL_ELEM_ID_NAME_MAXLEN];
	char dai_name[SNDRV_CTL_ELEM_ID_NAME_MAXLEN];
	uint32_t pcm_id;		/* unique ID - used to match */
	uint32_t dai_id;		/* unique ID - used to match */
	uint32_t playback;	/* supports playback mode */
	uint32_t capture;		/* supports capture mode */
	uint32_t compress;	/* 1 = compressed; 0 = PCM */
	struct snd_soc_tplg_stream stream[SND_SOC_TPLG_STREAM_CONFIG_MAX]; /* for DAI link */
	uint32_t num_streams;	/* number of streams */
	struct snd_soc_tplg_stream_caps caps[2]; /* playback and capture for DAI */
} __attribute__((packed));


/*
 * Describes the BE or CC link runtime supported configs or params
 *
 * File block representation for BE/CC link config :-
 * +-----------------------------------+-----+
 * | struct snd_soc_tplg_hdr           |  1  |
 * +-----------------------------------+-----+
 * | struct snd_soc_tplg_link_config   |  N  |
 * +-----------------------------------+-----+
 */
struct snd_soc_tplg_link_config {
	uint32_t size;            /* in bytes of this structure */
	uint32_t id;              /* unique ID - used to match */
	struct snd_soc_tplg_stream stream[SND_SOC_TPLG_STREAM_CONFIG_MAX]; /* supported configs playback and captrure */
	uint32_t num_streams;     /* number of streams */
} __attribute__((packed));
#endif
