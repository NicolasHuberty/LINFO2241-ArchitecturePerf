qemu-system-x86_64 -m 4096 -drive file=server.qcow2 -nographic -enable-kvm -netdev socket,id=right,listen=:1234 -device virtio-net-pci,netdev=right
