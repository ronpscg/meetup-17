#!/bin/bash
# Run with networking and a storage device, and add a USB dongle to the party
#init=/lib/systemd/systemd
#
init=/lib/systemd/systemd
#init=/bin/bash
sudo qemu-system-x86_64 --enable-kvm  -m 4g -smp 8 -kernel /home/ron/dev/out-june/target/product/pscg_debos/build-x86_64/kernel/linux-kernel-build/arch/x86/boot/bzImage \
	 -append "nokaslr root=/dev/vda rw init=$init  console=ttyS0  -fbcon=nodefer"   -serial mon:stdio  -display gtk,gl=on,show-cursor=on -device virtio-gpu -vga none -drive file=$HOME/demos/meetup17/wip.img,format=raw,if=virtio -device usb-ehci,id=usbbus \
	 -device usb-host,vendorid=0x0bda,productid=0xc820,bus=usbbus.0 

# Note: we could use something like this: HOWEVER,  if you remove a device and than plug it back in, it will not necessarily find itself under ths same path, and your guest
# 	will not have a way to know this [you could go to QEMU's monitor and attach it yourself...]
#	 -device usb-host,hostdevice=/dev/bus/usb/003/073,bus=usbbus.0 
#
#
