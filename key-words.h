/*
 * Key-words header file for trassembler.c
 */

#pragma once

#define INSTRUCTIONS 44
#define KEY_WORDS 53

#define loa 0x10
#define lra 0x11
#define lob 0x14
#define lrb 0x15
#define lia 0x12
#define lar 0x16
#define lai 0x17
#define lri 0x19
#define lir 0x2b
#define doi 0x2f
#define dai 0x1f
#define lax 0x20
#define lrx 0x21
#define lox 0x22
#define lxr 0x23
#define lxa 0x24
#define pra 0x25
#define par 0x26
#define prb 0x29
#define dxr 0x1d
#define dar 0x1b
#define and 0x01
#define ora 0x02
#define xor 0x03
#define add	0x04
#define sub 0x05
#define ush 0x06
#define dsh 0x07
#define cmp 0x2a
#define jmp 0x08
#define baz 0x09
#define ban 0x0a
#define bng 0x0b
#define bps 0x0c
#define bcr	0x0d
#define bxz 0x0e
#define bxn 0x0f
#define jsr 0x33
#define rts 0x34
#define pha 0x35
#define pla 0x36
#define inc 0x30
#define dec 0x31
#define nop 0x00

#define org 0x40
#define str 0x41
#define var 0x42
#define use 0x43
#define lab 0x44
#define b32 0x45
#define b64 0x46
#define f32 0x47
#define f64 0x48

char key_words[KEY_WORDS][4] = {
	"loa",
	"lra",
	"lob",
	"lrb",
	"lia",
	"lar",
	"lai",
	"lri",
	"lir",
	"doi",
	"dai",
	"lax",
	"lrx",
	"lox",
	"lxr",
	"lxa",
	"pra",
	"par",
	"prb",
	"dxr",
	"dar",
	"and",
	"ora",
	"xor",
	"add",
	"sub",
	"ush",
	"dsh",
	"cmp",
	"jmp",
	"baz",
	"ban",
	"bng",
	"bps",
	"bcr",
	"bxz",
	"bxn",
	"jsr",
	"rts",
	"pha",
	"pla",
	"inc",
	"dec",
	"nop",
	// dotwords:
	"org",
	"str",
	"var",
	"use",
	"lab",
	"b32",
	"b64",
	"f32",
	"f64"
};

char instr_code[INSTRUCTIONS] = {
	loa,
	lra,
	lob,
	lrb,
	lia,
	lar,
	lai,
	lri,
	lir,
	doi,
	dai,
	lax,
	lrx,
	lox,
	lxr,
	lxa,
	pra,
	par,
	prb,
	dxr,
	dar,
	and,
	ora,
	xor,
	add,
	sub,
	ush,
	dsh,
	cmp,
	jmp,
	baz,
	ban,
	bng,
	bps,
	bcr,
	bxz,
	bxn,
	jsr,
	rts,
	pha,
	pla,
	inc,
	dec,
	nop
};

char dot_code[KEY_WORDS - INSTRUCTIONS] = {
	org,
	str,
	var,
	use,
	lab,
	b32,
	b64,
	f32,
	f64
};
