# Onkyo/TEAC reverse engineering tools

This tools apply to the DAC-HA300 and HA-P90SD,
also some tools are generic blackfin tools.

Example:
./onkyo_teac_tools/descramble HAP90-b59.101 HAP90-b59.101.decrypted
(the -s 24 specifies a starting offset)
./onkyo_teac_tools/bfin_boot -s 24 -d HAP90-b108.130.decrypted -o fw_B108.130/
