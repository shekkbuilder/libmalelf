/*
 * The libmalelf is an evil library that could be used for good! It was
 * developed with the intent to assist in the process of infecting
 * binaries and provide a safe way to analyze malwares.
 *
 * Evil using this library is the responsibility of the programmer.
 *
 * Author:
 *         Tiago Natel de Moura <natel@secplus.com.br>
 *
 * Contributors:
 *         Daniel Ricardo dos Santos <danielricardo.santos@gmail.com>
 *         Paulo Leonardo Benatto    <benatto@gmail.com>
 *
 * Copyright 2012, 2013 by Tiago Natel de Moura. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <malelf/error.h>
#include <malelf/binary.h>

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

static void malelf_binary_open_mmap_TEST(void)
{
        MalelfBinary bin;
        _i32 result;

        malelf_binary_init(&bin);

        result = malelf_binary_open("bintest/uninfected", &bin);

        CU_ASSERT(result == MALELF_SUCCESS);
        CU_ASSERT(NULL != bin.fname);
        CU_ASSERT(bin.fd > 2);
        CU_ASSERT(NULL != bin.mem);
        CU_ASSERT(bin.size > 0);
        CU_ASSERT(NULL != MALELF_ELF_DATA(&bin.ehdr));
        CU_ASSERT(NULL != MALELF_ELF_DATA(&bin.phdr));
        CU_ASSERT(NULL != MALELF_ELF_DATA(&bin.shdr));
        CU_ASSERT(bin.alloc_type == MALELF_ALLOC_MMAP);
        CU_ASSERT(bin.class == MALELF_ELF32 ||
                  bin.class == MALELF_ELF64);
        result = malelf_binary_close(&bin);

        CU_ASSERT(result == MALELF_SUCCESS);
        CU_ASSERT(NULL == bin.fname);
        CU_ASSERT(bin.fd == -1);
        CU_ASSERT(NULL == bin.mem);
        CU_ASSERT(bin.size == 0);
        CU_ASSERT(NULL == MALELF_ELF_DATA(&bin.ehdr));
        CU_ASSERT(NULL == MALELF_ELF_DATA(&bin.phdr));
        CU_ASSERT(NULL == MALELF_ELF_DATA(&bin.shdr));
        CU_ASSERT(bin.alloc_type == MALELF_ALLOC_NONE);
        CU_ASSERT(bin.class == MALELF_ELFNONE);

        malelf_binary_init(&bin);

        /* Should fail */
        result = malelf_binary_open("/wrong/path/uninfected", &bin);
        CU_ASSERT(result == MALELF_ENOENT);
        CU_ASSERT(NULL == bin.fname);
        CU_ASSERT(bin.fd == -1);
        CU_ASSERT(NULL == bin.mem);
        CU_ASSERT(bin.size == 0);
        CU_ASSERT(NULL == MALELF_ELF_DATA(&bin.ehdr));
        CU_ASSERT(NULL == MALELF_ELF_DATA(&bin.phdr));
        CU_ASSERT(NULL == MALELF_ELF_DATA(&bin.shdr));
        CU_ASSERT(bin.alloc_type == MALELF_ALLOC_MMAP);
        CU_ASSERT(bin.class == MALELF_ELFNONE);

        result = malelf_binary_close(&bin);

        /* munmap on a non allocated memory area. */
        CU_ASSERT(result == MALELF_EINVAL);
}

static void malelf_binary_open_malloc_TEST(void)
{
        MalelfBinary bin;
        _i32 result;

        malelf_binary_init(&bin);
        malelf_binary_set_alloc_type(&bin, MALELF_ALLOC_MALLOC);

        result = malelf_binary_open("bintest/uninfected", &bin);
        CU_ASSERT(result == MALELF_SUCCESS);
        CU_ASSERT(NULL != bin.fname);
        CU_ASSERT(bin.fd > 2);
        CU_ASSERT(NULL != bin.mem);
        CU_ASSERT(bin.size > 0);
        CU_ASSERT(NULL != MALELF_ELF_DATA(&bin.ehdr));
        CU_ASSERT(NULL != MALELF_ELF_DATA(&bin.phdr));
        CU_ASSERT(NULL != MALELF_ELF_DATA(&bin.shdr));
        CU_ASSERT(bin.alloc_type == MALELF_ALLOC_MALLOC);
        CU_ASSERT(bin.class == MALELF_ELF32 ||
                  bin.class == MALELF_ELF64);

        result = malelf_binary_close(&bin);
        CU_ASSERT(result == MALELF_SUCCESS);

        malelf_binary_init(&bin);
        malelf_binary_set_alloc_type(&bin, MALELF_ALLOC_MALLOC);

        result = malelf_binary_open("/wrong/path/uninfected", &bin);
        CU_ASSERT(result == MALELF_ENOENT);
        CU_ASSERT(NULL == bin.fname);
        CU_ASSERT(bin.fd == -1);
        CU_ASSERT(NULL == bin.mem);
        CU_ASSERT(bin.size == 0);
        CU_ASSERT(NULL == MALELF_ELF_DATA(&bin.ehdr));
        CU_ASSERT(NULL == MALELF_ELF_DATA(&bin.phdr));
        CU_ASSERT(NULL == MALELF_ELF_DATA(&bin.shdr));
        CU_ASSERT(bin.alloc_type == MALELF_ALLOC_MALLOC);
        CU_ASSERT(bin.class == MALELF_ELFNONE);

        result = malelf_binary_close(&bin);
        CU_ASSERT(result == MALELF_SUCCESS);

        CU_ASSERT(NULL == bin.fname);
        CU_ASSERT(bin.fd == -1);
        CU_ASSERT(NULL == bin.mem);
        CU_ASSERT(bin.size == 0);
        CU_ASSERT(NULL == MALELF_ELF_DATA(&bin.ehdr));
        CU_ASSERT(NULL == MALELF_ELF_DATA(&bin.phdr));
        CU_ASSERT(NULL == MALELF_ELF_DATA(&bin.shdr));
        CU_ASSERT(bin.alloc_type == MALELF_ALLOC_NONE);
        CU_ASSERT(bin.class == MALELF_ELFNONE);
}

static void malelf_binary_get_section_name_TEST()
{
        MalelfBinary bin;
        _i32 result;
        char *name = NULL;

        malelf_binary_init(&bin);

        result = malelf_binary_open("bintest/uninfected", &bin);

        CU_ASSERT(result == MALELF_SUCCESS);
        CU_ASSERT(NULL != bin.fname);

        result = malelf_binary_get_section_name(&bin, 1, &name);
        CU_ASSERT(MALELF_SUCCESS == result);

        CU_ASSERT_STRING_EQUAL(".interp", name);

        result = malelf_binary_get_section_name(&bin, 2, &name);
        CU_ASSERT(MALELF_SUCCESS == result);
        CU_ASSERT_STRING_EQUAL(".note.ABI-tag", name);

        result = malelf_binary_get_section_name(&bin, 12, &name);
        CU_ASSERT(MALELF_SUCCESS == result);
        CU_ASSERT_STRING_EQUAL(".init", name);

        malelf_binary_close(&bin);
}

static void malelf_binary_get_section_TEST()
{
        MalelfBinary bin;
        _i32 result;
        MalelfSection section;

        malelf_binary_init(&bin);

        result = malelf_binary_open("bintest/uninfected", &bin);

        CU_ASSERT(result == MALELF_SUCCESS);
        CU_ASSERT(NULL != bin.fname);

        result = malelf_binary_get_section(&bin, 1, &section);
        CU_ASSERT(MALELF_SUCCESS == result);
        CU_ASSERT_STRING_EQUAL(section.name, ".interp");
        CU_ASSERT(section.offset == 0x134);
        CU_ASSERT(section.size == 0x13);
        CU_ASSERT(section.shdr != NULL);

        result = malelf_binary_get_section(&bin, 2, &section);
        CU_ASSERT(MALELF_SUCCESS == result);
        CU_ASSERT_STRING_EQUAL(section.name, ".note.ABI-tag");
        CU_ASSERT(section.offset == 0x148);
        CU_ASSERT(section.size == 0x20);
        CU_ASSERT(section.shdr != NULL);

        result = malelf_binary_get_section(&bin, 14, &section);
        CU_ASSERT(MALELF_SUCCESS == result);
        CU_ASSERT_STRING_EQUAL(section.name, ".text");
        CU_ASSERT(section.offset == 0x320);
        CU_ASSERT(section.size == 0x184);
        CU_ASSERT(section.shdr != NULL);

        malelf_binary_close(&bin);
}

void malelf_binary_create_elf_exec32_TEST()
{
        int error = MALELF_SUCCESS;
        MalelfBinary bin;
        Elf32_Ehdr *ehdr = NULL;

        malelf_binary_init(&bin);

        error = malelf_binary_create_elf_exec32(&bin);

        CU_ASSERT(MALELF_SUCCESS == error);

        ehdr = bin.ehdr.uhdr.h32;

        CU_ASSERT(ehdr->e_ident[0] == ELFMAG0);
        CU_ASSERT(ehdr->e_ident[1] == ELFMAG1);
        CU_ASSERT(ehdr->e_ident[2] == ELFMAG2);
        CU_ASSERT(ehdr->e_ident[3] == ELFMAG3);
        CU_ASSERT(ehdr->e_ident[4] == ELFCLASS32);
        CU_ASSERT(ehdr->e_ident[5] == ELFDATA2LSB);
        CU_ASSERT(ehdr->e_ident[6] == EV_CURRENT);
        CU_ASSERT(ehdr->e_ident[7] == ELFOSABI_LINUX);
        CU_ASSERT(ehdr->e_ident[8] == 0);
        CU_ASSERT(ehdr->e_ident[9] == 0);
        CU_ASSERT(ehdr->e_ident[10] == 0);
        CU_ASSERT(ehdr->e_ident[11] == 0);
        CU_ASSERT(ehdr->e_ident[12] == 0);
        CU_ASSERT(ehdr->e_ident[13] == 0);
        CU_ASSERT(ehdr->e_ident[14] == 0);
        CU_ASSERT(ehdr->e_ident[15] == 0);

        /* executable file */
        CU_ASSERT(ehdr->e_type == ET_EXEC);
        CU_ASSERT(ehdr->e_machine == EM_386);
        CU_ASSERT(ehdr->e_version == EV_CURRENT);
        CU_ASSERT(ehdr->e_entry == 0x00);
        CU_ASSERT(ehdr->e_phoff == 0x00);
        CU_ASSERT(ehdr->e_shoff == 0x00);
        CU_ASSERT(ehdr->e_flags == 0x00);
        CU_ASSERT(ehdr->e_ehsize == sizeof (Elf32_Ehdr)); // 52 bytes
        CU_ASSERT(ehdr->e_phentsize == 0x00);
        CU_ASSERT(ehdr->e_phnum == 0x00);
        CU_ASSERT(ehdr->e_shentsize == 0x00);
        CU_ASSERT(ehdr->e_shnum == 0x00);
        CU_ASSERT(ehdr->e_shstrndx == SHN_UNDEF);

        malelf_binary_close(&bin);
}

void malelf_binary_create_elf_exec64_TEST()
{
        int error = MALELF_SUCCESS;
        MalelfBinary bin;
        Elf64_Ehdr *ehdr = NULL;

        malelf_binary_init(&bin);

        error = malelf_binary_create_elf_exec64(&bin);

        CU_ASSERT(MALELF_SUCCESS == error);

        ehdr = bin.ehdr.uhdr.h64;

        CU_ASSERT(ehdr->e_ident[0] == ELFMAG0);
        CU_ASSERT(ehdr->e_ident[1] == ELFMAG1);
        CU_ASSERT(ehdr->e_ident[2] == ELFMAG2);
        CU_ASSERT(ehdr->e_ident[3] == ELFMAG3);
        CU_ASSERT(ehdr->e_ident[4] == ELFCLASS64);
        CU_ASSERT(ehdr->e_ident[5] == ELFDATA2LSB);
        CU_ASSERT(ehdr->e_ident[6] == EV_CURRENT);
        CU_ASSERT(ehdr->e_ident[7] == ELFOSABI_LINUX);
        CU_ASSERT(ehdr->e_ident[8] == 0);
        CU_ASSERT(ehdr->e_ident[9] == 0);
        CU_ASSERT(ehdr->e_ident[10] == 0);
        CU_ASSERT(ehdr->e_ident[11] == 0);
        CU_ASSERT(ehdr->e_ident[12] == 0);
        CU_ASSERT(ehdr->e_ident[13] == 0);
        CU_ASSERT(ehdr->e_ident[14] == 0);
        CU_ASSERT(ehdr->e_ident[15] == 0);

        /* executable file */
        CU_ASSERT(ehdr->e_type == ET_EXEC);
        CU_ASSERT(ehdr->e_machine == EM_X86_64);
        CU_ASSERT(ehdr->e_version == EV_CURRENT);
        CU_ASSERT(ehdr->e_entry == 0x00);
        CU_ASSERT(ehdr->e_phoff == 0x00);
        CU_ASSERT(ehdr->e_shoff == 0x00);
        CU_ASSERT(ehdr->e_flags == 0x00);
        CU_ASSERT(ehdr->e_ehsize == sizeof (Elf64_Ehdr)); // 52 bytes
        CU_ASSERT(ehdr->e_phentsize == 0x00);
        CU_ASSERT(ehdr->e_phnum == 0x00);
        CU_ASSERT(ehdr->e_shentsize == 0x00);
        CU_ASSERT(ehdr->e_shnum == 0x00);
        CU_ASSERT(ehdr->e_shstrndx == SHN_UNDEF);

        malelf_binary_close(&bin);
}

void malelf_binary_add_phdr32_TEST()
{
        int error = MALELF_SUCCESS;
        MalelfBinary bin;
        Elf32_Phdr new_phdr, *phdr = NULL;
        MalelfPhdr mphdr;

        malelf_binary_init(&bin);

        error = malelf_binary_create_elf_exec32(&bin);
        CU_ASSERT(MALELF_SUCCESS == error);

        /* First, configure your executable (PT_LOAD) segment */
        new_phdr.p_type = PT_PHDR;
        new_phdr.p_offset = sizeof (Elf32_Ehdr);
        new_phdr.p_vaddr = new_phdr.p_paddr = 0x08048000 +
                new_phdr.p_offset;
        new_phdr.p_filesz = new_phdr.p_memsz = sizeof(Elf32_Phdr);
        new_phdr.p_flags = PF_R | PF_X;
        new_phdr.p_align = 0;

        error = malelf_binary_add_phdr32(&bin, &new_phdr);
        CU_ASSERT(MALELF_SUCCESS == error);

        _u32 uvalue;
        error = malelf_ehdr_get_phoff(&bin.ehdr, &uvalue);
        CU_ASSERT(MALELF_SUCCESS == error);
        CU_ASSERT(uvalue == new_phdr.p_offset);

        error = malelf_ehdr_get_phnum(&bin.ehdr, &uvalue);
        CU_ASSERT(MALELF_SUCCESS == error);
        CU_ASSERT(uvalue == 1);

        error = malelf_phdr_get_type(&bin.phdr, &uvalue, 0);
        CU_ASSERT(MALELF_SUCCESS == error);
        CU_ASSERT(PT_PHDR == uvalue);

        error = malelf_phdr_get_vaddr(&bin.phdr, &uvalue, 0);
        CU_ASSERT(MALELF_SUCCESS == error);
        CU_ASSERT(uvalue == new_phdr.p_vaddr);

        error = malelf_phdr_get_paddr(&bin.phdr, &uvalue, 0);
        CU_ASSERT(MALELF_SUCCESS == error);
        CU_ASSERT(uvalue == new_phdr.p_paddr);

        error = malelf_phdr_get_memsz(&bin.phdr, &uvalue, 0);
        CU_ASSERT(MALELF_SUCCESS == error);
        CU_ASSERT(uvalue == new_phdr.p_memsz);

        error = malelf_phdr_get_flags(&bin.phdr, &uvalue, 0);
        CU_ASSERT(MALELF_SUCCESS == error);
        CU_ASSERT(uvalue == new_phdr.p_flags);

        error = malelf_phdr_get_align(&bin.phdr, &uvalue, 0);
        CU_ASSERT(MALELF_SUCCESS == error);
        CU_ASSERT(uvalue == new_phdr.p_align);

        error = malelf_phdr_get_offset(&bin.phdr, &uvalue,  0);
        CU_ASSERT(MALELF_SUCCESS == error);
        CU_ASSERT(uvalue == new_phdr.p_offset);

        error = malelf_phdr_get_filesz(&bin.phdr, &uvalue, 0);
        CU_ASSERT(MALELF_SUCCESS == error);
        CU_ASSERT(uvalue == new_phdr.p_filesz);

        error = malelf_binary_get_phdr(&bin, &mphdr);
        CU_ASSERT(MALELF_SUCCESS == error);

        CU_ASSERT(NULL != (void*)&mphdr);

        phdr = mphdr.uhdr.h32;
        CU_ASSERT(NULL != phdr);
        CU_ASSERT(MALELF_ELF32 == mphdr.class);

        CU_ASSERT(new_phdr.p_type == phdr->p_type);
        CU_ASSERT(new_phdr.p_offset == phdr->p_offset);
        CU_ASSERT(new_phdr.p_vaddr == phdr->p_vaddr);
        CU_ASSERT(new_phdr.p_paddr == phdr->p_paddr);
        CU_ASSERT(new_phdr.p_filesz == phdr->p_filesz);
        CU_ASSERT(new_phdr.p_flags == phdr->p_flags);
        CU_ASSERT(new_phdr.p_memsz == phdr->p_memsz);
        CU_ASSERT(new_phdr.p_align == phdr->p_align);

        malelf_binary_close(&bin);
}

void malelf_binary_write_TEST()
{
        int error = MALELF_SUCCESS;
        MalelfBinary bin, bin2;
        struct stat st_info;

        malelf_binary_init(&bin);

        error = malelf_binary_open("bintest/uninfected", &bin);
        CU_ASSERT(MALELF_SUCCESS == error);

        error = malelf_binary_write(&bin, "bintest/uninfected_copy", 1);
        CU_ASSERT(MALELF_SUCCESS == error);

        CU_ASSERT(0 == stat("bintest/uninfected_copy", &st_info));
        CU_ASSERT(st_info.st_size > 0);
        CU_ASSERT(st_info.st_size == bin.size);

        malelf_binary_init(&bin2);
        error = malelf_binary_open("bintest/uninfected_copy", &bin2);
        CU_ASSERT(MALELF_SUCCESS == error);

        CU_ASSERT(bin2.size == bin.size);
        CU_ASSERT(bin2.class == bin.class);
        CU_ASSERT(bin.alloc_type == bin2.alloc_type);

        int i;

        switch (bin2.class) {
        case MALELF_ELF32: {
                Elf32_Ehdr *ehdr = (Elf32_Ehdr *) MALELF_ELF_DATA(&bin.ehdr);
                Elf32_Ehdr *ehdr2 = (Elf32_Ehdr *) MALELF_ELF_DATA(&bin2.ehdr);

                CU_ASSERT(NULL != ehdr2);
                CU_ASSERT(NULL != ehdr);

                for (i = 0; i < 16; i++) {
                        CU_ASSERT(ehdr->e_ident[i] == ehdr2->e_ident[i]);
                }

                CU_ASSERT(ehdr->e_type == ehdr2->e_type);
                CU_ASSERT(ehdr->e_machine == ehdr2->e_machine);
                CU_ASSERT(ehdr->e_version == ehdr2->e_version);
                CU_ASSERT(ehdr->e_entry == ehdr2->e_entry);
                CU_ASSERT(ehdr->e_phoff == ehdr2->e_phoff);
                CU_ASSERT(ehdr->e_shoff == ehdr2->e_shoff);
                CU_ASSERT(ehdr->e_flags == ehdr2->e_flags);
                CU_ASSERT(ehdr->e_ehsize == ehdr2->e_ehsize);
                CU_ASSERT(ehdr->e_phentsize == ehdr2->e_phentsize);
                CU_ASSERT(ehdr->e_phnum == ehdr2->e_phnum);
                CU_ASSERT(ehdr->e_shentsize == ehdr2->e_shentsize);
                CU_ASSERT(ehdr->e_shnum == ehdr2->e_shnum);
                CU_ASSERT(ehdr->e_shstrndx == ehdr2->e_shstrndx);

                break;
        }
        case MALELF_ELF64: {
                Elf64_Ehdr *ehdr = (Elf64_Ehdr *) MALELF_ELF_DATA(&bin.ehdr);
                Elf64_Ehdr *ehdr2 = (Elf64_Ehdr *) MALELF_ELF_DATA(&bin2.ehdr);

                CU_ASSERT(NULL != ehdr2);
                CU_ASSERT(NULL != ehdr);

                for (i = 0; i < 16; i++) {
                        CU_ASSERT(ehdr->e_ident[i] == ehdr2->e_ident[i]);
                }

                CU_ASSERT(ehdr->e_type == ehdr2->e_type);
                CU_ASSERT(ehdr->e_machine == ehdr2->e_machine);
                CU_ASSERT(ehdr->e_version == ehdr2->e_version);
                CU_ASSERT(ehdr->e_entry == ehdr2->e_entry);
                CU_ASSERT(ehdr->e_phoff == ehdr2->e_phoff);
                CU_ASSERT(ehdr->e_shoff == ehdr2->e_shoff);
                CU_ASSERT(ehdr->e_flags == ehdr2->e_flags);
                CU_ASSERT(ehdr->e_ehsize == ehdr2->e_ehsize);
                CU_ASSERT(ehdr->e_phentsize == ehdr2->e_phentsize);
                CU_ASSERT(ehdr->e_phnum == ehdr2->e_phnum);
                CU_ASSERT(ehdr->e_shentsize == ehdr2->e_shentsize);
                CU_ASSERT(ehdr->e_shnum == ehdr2->e_shnum);
                CU_ASSERT(ehdr->e_shstrndx == ehdr2->e_shstrndx);

                break;
        }
        default:
                CU_ASSERT(0);
        }

        CU_ASSERT(memcmp(bin.mem, bin2.mem, bin2.size) == 0);

        malelf_binary_close(&bin);
        malelf_binary_close(&bin2);
        unlink("bintest/uninfected_copy");
}


CU_ErrorCode binary_get_test_suite(CU_pSuite *rsuite)
{
        CU_pSuite suite = NULL;

        if (NULL == rsuite) {
                return -1;
        }

        suite = CU_add_suite("Class Binary", NULL, NULL);
        if(NULL == suite) {
                *rsuite = NULL;
                return CU_get_error();
        }

        if ((NULL == CU_add_test(suite,
                                 "malelf_binary_open_mmap_TEST",
                                 malelf_binary_open_mmap_TEST)) ||
            (NULL == CU_add_test(suite,
                                 "malelf_binary_open_malloc_TEST",
                                 malelf_binary_open_malloc_TEST)) ||
            (NULL == CU_add_test(suite,
                                 "malelf_binary_get_section_name_TEST",
                                 malelf_binary_get_section_name_TEST)) ||
            (NULL == CU_add_test(suite,
                                 "malelf_binary_get_section_TEST",
                                 malelf_binary_get_section_TEST)) ||
            (NULL == CU_add_test(suite,
                                 "malelf_binary_write_TEST",
                                 malelf_binary_write_TEST)) ||
            (NULL == CU_add_test(suite,
                                 "malelf_binary_create_elf_exec32_TEST",
                                 malelf_binary_create_elf_exec32_TEST))
            || (NULL == CU_add_test(suite,
                                    "malelf_binary_create_elf_exec64_TEST",
                                    malelf_binary_create_elf_exec64_TEST)) || (NULL == CU_add_test(suite,                                                                   "malelf_binary_add_phdr32_TEST",                                  malelf_binary_add_phdr32_TEST))) {
                *rsuite = NULL;
                return CU_get_error();
        }

        *rsuite = suite;
        return CUE_SUCCESS;
}
