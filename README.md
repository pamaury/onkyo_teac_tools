# Onkyo/TEAC reverse engineering tools

These tools apply to the DAC-HA300 and HA-P90SD,
also some tools are generic blackfin tools.

Example:

    $ ./onkyo_teac_tools/descramble ../firmware/HAP90-b108.130 extracted-bootstream

    $ ./onkyo_teac_tools/bfin_boot -d extracted-bootstream -o fw_B108.130/

    $ file fw_B108.130/*
    fw_B108.130/0.elf: ELF 32-bit LSB executable, Analog Devices Blackfin, version 1 (SYSV), statically linked, stripped
    fw_B108.130/1.elf: ELF 32-bit LSB executable, Analog Devices Blackfin, version 1 (SYSV), statically linked, stripped

    $ ./onkyo_teac_tools/scramble extracted-bootstream > generated-image

    $ md5sum ../firmware/HAP90-b108.130 generated-image
    34dfd73373f6b9f92058201cd56b80a0  ../firmware/HAP90-b108.130
    34dfd73373f6b9f92058201cd56b80a0  generated-image

Note that the sums won't match for the other images due to versioning
information in the header.  This info appears to get displayed but
otherwise ignored by the firmware update program.
