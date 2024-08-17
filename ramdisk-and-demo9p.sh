#!/bin/bash
qemu-system-x86_64  -m 1g -smp 4 -kernel $KERNEL_OUT/arch/x86/boot/bzImage -enable-kvm -initrd ${RAMDISK_PACKED}   -append "nokaslr rdinit=/init console=ttyS0  -fbcon=nodefer"  -serial mon:stdio  \
	-display gtk,gl=on -vga none -device virtio-gpu \
	-virtfs local,path=/home/ron/demos/meetup17/host-mount-for-PSCG-mini-linux,mount_tag=thepscg-mounts-you,security_model=none 

# To chroot into something fuller (a Debian package with some examples under demo/ use the following instead:
# -virtfs local,path=/home/ron/demos/meetup17/wip-rootfs,mount_tag=thepscg-mounts-you,security_model=none
