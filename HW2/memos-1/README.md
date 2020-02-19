First Deliverable steps:

1) Created the raw image: qemu-img create -f raw c1.img 30240K (60 x 16 x 63 x 512 as indicated by the BOCHS HOW-TO)
2) In order to create the image run the compile.sh script
3) Run QEMU: qemu-system-i386 -m 16 -hda memos-1_test
