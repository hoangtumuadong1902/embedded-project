Driver: ads1115_driver  
Members:  
    Nguyen Huynh Dang Khoa 22146153
    Pham Vu Nhat Huy       22146128
    Mai Xuan Khoa          22146152


Commands to Use
-----------------------
make create_overlay      # Compile and copy the device tree overlay
make enable_overlay      # Add overlay to /boot/config.txt
make create_drive        # Build the kernel module
make install_drive       # Install the module to the system
make end_reboot          # Reboot the system to apply changes
make remove_overlay      # Remove the overlay
make uninstall_drive     # Uninstall the kernel module
make end_reboot          # Reboot the system (after overlay or module changes)
make test                # Test overlay creation and activation

How to Load the Driver  
-----------------------
make create_overlay      # Compile and copy the device tree overlay
make enable_overlay      # Add overlay to /boot/config.txt
make create_drive        # Build the kernel module
make install_drive       # Install the module to the system
make end_reboot          # Reboot the system to apply changes

