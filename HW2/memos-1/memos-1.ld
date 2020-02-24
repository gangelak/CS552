OUTPUT_FORMAT("elf32-i386")
/*OUTPUT_FORMAT("binary")*/
OUTPUT_ARCH(i386)
ENTRY(_start)
SECTIONS
{
  .foo : { *(.*) }
}
