#!/bin/sh
##Client server
cmdClient="qemu-system-x86_64 -m 4096 -drive file=client.qcow2 -nographic -enable-kvm -netdev user,id=mynet0 -device virtio-net-pci,netdev=mynet0 -netdev socket,id=left,listen=:1234 -device virtio-net-pci,netdev=left"
cmdRouter="qemu-system-x86_64 -m 4096 -drive file=router.qcow2 -nographic -enable-kvm -netdev user,id=mynet0 -device virtio-net-pci,netdev=mynet0 -netdev socket,id=left,listen=:1234 -device virtio-net-pci,netdev=left -netdev socket,id=right,listen=:1235 -device virtio-net-pci,netdev=right "
cmdServer="qemu-system-x86_64 -m 4096 -drive file=server.qcow2 -nographic -enable-kvm -netdev user,id=mynet0 -device virtio-net-pci,netdev=mynet0 -netdev socket,id=right,listen=:1235 -device virtio-net-pci,netdev=right" 
title1="Client"
title2="Router"
title3="Server"
gnome-terminal --tab --title="$title1" --command="bash -c '$cmdClient; $SHELL'" \
               --tab --title="$title2" --command="bash -c '$cmdRouter; $SHELL'" \
               --tab --title="$title3" --command="bash -c '$cmdServer; $SHELL'" 