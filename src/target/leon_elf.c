/**
 * OOCD target for Leon (Sparc v8) CPUs
 * (C)2016 Roman Bartosinski, daiteq,s.r.o.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "leon.h"

/* -------------------------------------------------------------------------- */
static leon_elf_section_t *leon_elf_add_section(struct leon_common *pleon,
          int idx, char *name, uint32_t type, uint32_t flags,
          uint32_t addr, uint32_t size)
{
	leon_elf_section_t *psec = pleon->lelf_sects, *pprev = NULL;
	while (psec) {
		pprev = psec;
		psec = psec->next;
	}
	psec = malloc(sizeof(leon_elf_section_t));
	if (psec == NULL) return NULL;
	psec->index = idx;
	psec->name = strdup(name);
	psec->type = type;
	psec->flags = flags;
	psec->minaddr = addr;
	psec->maxaddr = addr+size-1;
	psec->next = NULL;
	if (pprev)
		pprev->next = psec;
	else
		pleon->lelf_sects = psec;
	pleon->lelf_nsecs++;
	return psec;
}

static leon_elf_symbol_t *leon_elf_add_symbol(struct leon_common *pleon, char *name,
          uint32_t value, uint32_t size, uint8_t info, uint8_t other, uint8_t sidx)
{
	leon_elf_symbol_t *psym = pleon->lelf_symbs, *pprev = NULL;
	while (psym) {
		pprev = psym;
		psym = psym->next;
	}
	psym = malloc(sizeof(leon_elf_symbol_t));
	if (psym == NULL) return NULL;
	psym->name = strdup(name);
	psym->value = value;
	psym->size = size;
	psym->info = info;
	psym->other = other;
	psym->secidx = sidx;
	psym->next = NULL;
	if (pprev)
		pprev->next = psym;
	else
		pleon->lelf_symbs = psym;
	pleon->lelf_nsyms++;
	return psym;
}

void leon_elf_reset_sections_symbols(struct leon_common *pleon)
{
	leon_elf_section_t *psec = pleon->lelf_sects;
	while (psec) {
		leon_elf_section_t *pn = psec->next;
		if (psec->name) free(psec->name);
		free(psec);
		psec = pn;
	}
	pleon->lelf_sects = NULL;
	pleon->lelf_nsecs = 0;

	leon_elf_symbol_t *psym = pleon->lelf_symbs;
	while (psym) {
		leon_elf_symbol_t *pn = psym->next;
		if (psym->name) free(psym->name);
		free(psym);
		psym = pn;
	}
	pleon->lelf_symbs = NULL;
	pleon->lelf_nsyms = 0;
}

char *leon_elf_val2sym(struct leon_common *pleon, const char *secname, uint32_t val)
{
	int sidx = -1;
	if (pleon==NULL || pleon->lelf_symbs==NULL) return NULL;
	if (secname) {
		if (pleon->lelf_sects==NULL) return NULL;
		leon_elf_section_t *psec = pleon->lelf_sects;
		while (psec) {
			if (!strcmp(secname, psec->name)) {
				sidx = psec->index;
				break;
			}
			psec = psec->next;
		}
	}
	leon_elf_symbol_t *psym = pleon->lelf_symbs;
	while (psym) {
		if (sidx<0 || sidx==psym->secidx) {
			/* TODO: add check of visibility and type of symbol */
//LOG_INFO(" - sym: '%s'", psym->name);
			if ((ELF32_ST_TYPE(psym->info)==STT_NOTYPE) &&
			    (psym->value==val))
				return psym->name;
		}
		psym = psym->next;
	}
	return NULL;
}

leon_elf_symbol_t *leon_elf_sym2val(struct leon_common *pleon, const char *secname, const char *sym, uint32_t *val)
{
	int sidx = -1;
	if (pleon==NULL || pleon->lelf_symbs==NULL) return NULL;
	if (secname) {
		if (pleon->lelf_sects==NULL) return NULL;
		leon_elf_section_t *psec = pleon->lelf_sects;
		while (psec) {
			if (!strcmp(secname, psec->name)) {
				sidx = psec->index;
				break;
			}
			psec = psec->next;
		}
	}
	leon_elf_symbol_t *psym = pleon->lelf_symbs;
	while (psym) {
		if (sidx<0 || sidx==psym->secidx) {
			if (!strcmp(sym, psym->name)) {
				if (val) *val = psym->value;
				return psym;
			}
		}
		psym = psym->next;
	}
	return NULL;
}

/* !!! the function returns only the first section */
leon_elf_section_t *leon_elf_addr2sec(struct leon_common *pleon, uint32_t addr)
{
	leon_elf_section_t *ps = pleon->lelf_sects;
	if (pleon->lelf_nsecs<=0) return NULL;
	while (ps) {
//printf(" ... '%s' : 0x%X - 0x%X (0x%X)\n", ps->name, ps->minaddr, ps->maxaddr, ps->flags);
		if ((ps->type == SHT_PROGBITS || ps->type == SHT_NOBITS) &&
		    addr>=ps->minaddr && addr<=ps->maxaddr)
		  return ps;
		ps = ps->next;
	}
	return NULL;
}

/* -------------------------------------------------------------------------- */
#define elf_field16(elf, field) \
	((elf->endianness == ELFDATA2LSB) ? \
	le_to_h_u16((uint8_t *)&field) : be_to_h_u16((uint8_t *)&field))

#define elf_field32(elf, field) \
	((elf->endianness == ELFDATA2LSB) ? \
	le_to_h_u32((uint8_t *)&field) : be_to_h_u32((uint8_t *)&field))

static int leon_load_elf_section(struct image_elf *pie, int sidx,
                            Elf32_Shdr **psect, void **pctx, uint32_t *ctxsz)
{
	size_t rdsz;
	size_t ssz = elf_field16(pie, pie->header->e_shentsize);
	int ofst = elf_field32(pie, pie->header->e_shoff) + ssz * sidx;
	Elf32_Shdr *pnsh = (Elf32_Shdr *)realloc(*psect, ssz);
	if (pnsh==NULL) return ERROR_FAIL;
	*psect = pnsh;
	int retval = fileio_seek(pie->fileio, ofst);
	if (retval != ERROR_OK) return retval;
	retval = fileio_read(pie->fileio, ssz, pnsh, &rdsz);
	if (retval != ERROR_OK || rdsz != ssz) {
		return retval;
	}
	if (pctx) { /* load also context */
		size_t csz = elf_field32(pie, pnsh->sh_size);
		void *pb = realloc(*pctx, csz);
		if (pb==NULL) return ERROR_FAIL;
		if (ctxsz) *ctxsz = csz;
		*pctx = pb;
		retval = fileio_seek(pie->fileio, elf_field32(pie, pnsh->sh_offset));
		if (retval != ERROR_OK) return retval;
		retval = fileio_read(pie->fileio, csz, pb, &rdsz);
		if (retval != ERROR_OK || rdsz != csz) {
			return retval;
		}
	}
	return retval;
}

static char *leon_elf_string(char *pss, uint32_t sssz, uint32_t ofst)
{
	if (pss==NULL || sssz==0) return "";
	if (ofst>=sssz) return "";
	return (pss + ofst);
}


int leon_load_elf_symbols(struct leon_common *leon, struct image *image)
{
	int retval, i, nsym;
	if (image->type!=IMAGE_ELF) return -1;
	/* BEGIN: loading symbols */
	leon_elf_reset_sections_symbols(leon);
	struct image_elf *pie = (struct image_elf *)image->type_private;
//		LOG_INFO(">> ELF type=x%X mach=x%X ver=x%X entry=0x%08X",
//		           elf_field16(pie, pie->header->e_type),
//		           elf_field16(pie, pie->header->e_machine),
//		           elf_field32(pie, pie->header->e_version),
//		           elf_field32(pie, pie->header->e_entry));
//		LOG_INFO("   Section header table @x%X  - esize=%u, enum=%u, stridx=%u",
//		          elf_field32(pie, pie->header->e_shoff),
//		          elf_field16(pie, pie->header->e_shentsize),
//		          elf_field16(pie, pie->header->e_shnum),
//		          elf_field16(pie, pie->header->e_shstrndx));
	leon->lelf_epoint = elf_field32(pie, pie->header->e_entry);

	Elf32_Shdr *bsec = NULL;
	char *bstr = NULL;
	uint32_t bstrsz = 0;
	do {
		/* read section with sh string table */
		retval = leon_load_elf_section(pie,
																	 elf_field16(pie, pie->header->e_shstrndx),
																	 &bsec, (void **) &bstr, &bstrsz);
		if (retval != ERROR_OK) break;
		/* read all sections */
		for (i=0; i<elf_field16(pie, pie->header->e_shnum); ++i) {
			retval = leon_load_elf_section(pie, i, &bsec, NULL, NULL);
			if (retval!=ERROR_OK) break;

//				LOG_INFO(" Read section header #%d", i);
//				LOG_INFO("  name = '%s' (%u) type=%u flags=x%X addr=x%X offs=x%X size=%u, link=%u, info=%u",
//				         leon_elf_string(bstr, bstrsz, elf_field32(pie, bsec->sh_name)),
//				         elf_field32(pie, bsec->sh_name), elf_field32(pie, bsec->sh_type),
//				         elf_field32(pie, bsec->sh_flags), elf_field32(pie, bsec->sh_addr),
//				         elf_field32(pie, bsec->sh_offset), elf_field32(pie, bsec->sh_size),
//				         elf_field32(pie, bsec->sh_link), elf_field32(pie, bsec->sh_info));
			if (elf_field32(pie, bsec->sh_type)==SHT_PROGBITS &&
					(elf_field32(pie, bsec->sh_flags) & SHF_ALLOC)) {
				LOG_INFO(" section: %s at 0x%08X, size %u bytes",
							leon_elf_string(bstr, bstrsz, elf_field32(pie, bsec->sh_name)),
							elf_field32(pie, bsec->sh_addr), elf_field32(pie, bsec->sh_size));
			}
			leon_elf_add_section(leon, i,
													 leon_elf_string(bstr, bstrsz, elf_field32(pie, bsec->sh_name)),
													 elf_field32(pie, bsec->sh_type),
													 elf_field32(pie, bsec->sh_flags),
													 elf_field32(pie, bsec->sh_addr),
													 elf_field32(pie, bsec->sh_size));
			if (elf_field32(pie, bsec->sh_type)==SHT_SYMTAB) {
				void *bstab = NULL;
				uint32_t bstsz = 0;
				Elf32_Shdr *bsecss = NULL;
				char *bsymstr = NULL;
				uint32_t bsssz = 0;
				do {
					retval = leon_load_elf_section(pie, i, &bsec, &bstab, &bstsz);
					if (retval!=ERROR_OK) break;
					retval = leon_load_elf_section(pie, elf_field32(pie, bsec->sh_link), &bsecss, (void *)&bsymstr, &bsssz);
					if (retval!=ERROR_OK) break;
					nsym = bstsz / elf_field32(pie, bsec->sh_entsize);
//						LOG_INFO("  - SYMTAB ... n = %d", nsym);
					Elf32_Sym *psym = (Elf32_Sym *)bstab;
					int n = nsym;
					while (n--) {
//							LOG_INFO(" #%d: '%s'  val=x%X  sz=%u  info=%u  other=%u  sec=x%X", n,
//							        leon_elf_string(bsymstr, bsssz, elf_field32(pie, psym->st_name)),
//							        elf_field32(pie, psym->st_value), elf_field32(pie, psym->st_size),
//							        psym->st_info, psym->st_other, elf_field16(pie, psym->st_shndx));
						leon_elf_add_symbol(leon, leon_elf_string(bsymstr, bsssz, elf_field32(pie, psym->st_name)),
										elf_field32(pie, psym->st_value), elf_field32(pie, psym->st_size),
										psym->st_info, psym->st_other, elf_field16(pie, psym->st_shndx));
						psym++;
					}
				} while(0);
				if (bstab) free(bstab);
				if (bsecss) free(bsecss);
				if (bsymstr) free(bsymstr);
			}
		}
	} while (0);
	if (bsec) free(bsec);
	if (bstr) free(bstr);
	/* END: loading symbols */
	return 0;
}

