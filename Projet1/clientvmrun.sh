qemu-system-x86_64 -m 4096 -drive file=client.qcow2 -nographic -enable-kvm -netdev socket,id=left,connect=:1234 -device virtio-net-pci,netdev=left
