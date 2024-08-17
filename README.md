# Meetup 17 working repository

The files in this repo are used in the series of videos (3 from the meetup itself, and much more from its preparation, that appear after them in the playlist) in https://www.youtube.com/watch?v=NQz6VqvtehI&list=PLBaH8x4hthVysdRTOlg2_8hL6CWCnN5l-&index=47

The videos have done **much more** than what is written below in this readme, but the files in the repo are accurate, and can be used.
If anyone actually uses them, except for me, please let me know, and you are of course welcome to contribute. 


The objective of this repo is to be a place where static binaries that I intend to demo in meetup #17 can be at, so you can reproduce the demos at your will.

As per the time of writing the idea is:
- To take *PSCG-mini-linux* in the branch last shown in 2020 (busybox-init, or something that comes out of it, as the `master` branch is used actively in training, does not concern itself with minimization, and builds for all architectures [or so I think, builds for x86_64, aarch64 and riscv64 for sure, and probably for the 32-bit variants of the former two])
- Either put some of the files here in a ramdisk, and pack it (although I suppose I will put a PPM file there, and it will be big [sorry about putting it in the repo if you use it, I always avoid these things but working on the structure for the meetup is taking a lot of time, and I cannot say it is very rewarding so just take it for what it is.
(yeah, I mean those who want to see stuff in github repos...)
- Or put in a folder and use 9p (but careful about it in general. for r/x it's excellent) - and mount it.


**README is definitely not updated, I will add some kernel configs, and maybe images, for backup (though that will be quite large)**
**You are welcome to help and keep this up to date**


## Running example:
DRM device:
```bash
qemu-system-x86_64  -m 1g -smp 4 -kernel $KERNEL_OUT/arch/x86/boot/bzImage -enable-kvm -initrd ${RAMDISK_PACKED}   -append "nokaslr rdinit=/init console=ttyS0  -fbcon=nodefer"  -serial mon:stdio -display gtk,gl=on -device virtio-gpu -vga none  -virtfs local,path=/home/ron/demos/meetup17/host-mount-for-PSCG-mini-linux/,mount_tag=thepscg-mounts-you,security_model=none
```

Bochs VGA device (which in the respective branch also exposes DRM)
```bash
qemu-system-x86_64  -m 1g -smp 4 -kernel $KERNEL_OUT/arch/x86/boot/bzImage -enable-kvm -initrd ${RAMDISK_PACKED}   -append "nokaslr rdinit=/init console=ttyS0  -fbcon=nodefer"  -serial mon:stdio -display gtk,gl=on -vga std -virtfs local,path=/home/ron/demos/meetup17/host-mount-for-PSCG-mini-linux/,mount_tag=thepscg-mounts-you,security_model=none
```

(at least as per the time of running) Cirrus with now framebuffer driver, e.g. "pure VGA/VESA console"
```bash
qemu-system-x86_64  -m 1g -smp 4 -kernel $KERNEL_OUT/arch/x86/boot/bzImage -enable-kvm -initrd ${RAMDISK_PACKED}   -append "nokaslr rdinit=/init console=ttyS0  -fbcon=nodefer"  -serial mon:stdio -display gtk -vga cirrus -virtfs local,path=/home/ron/demos/meetup17/host-mount-for-PSCG-mini-linux/,mount_tag=thepscg-mounts-you,security_model=none
```


## Note about DRM / VGA/ etc. (not going to explain much here)
You will have to listen to the lecutre itself to understand the subtleties in vga/drm in QEMU - as the latest commit did implement DRM for both Qlx and Bochs so it may be confusing (take them off, and your VGA will be VGA, and virtio-gpu will be your DRM device
