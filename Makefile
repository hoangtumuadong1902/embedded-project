# Module name (without .ko extension)
obj-m += ads1115_driver.o

# Path to the current kernel build directory
KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

# Build and install the kernel module
build_and_install:
	@echo ">> Cleaning old build files..."
	@rm -f *.o *.mod.c *.mod *.symvers *.order modules.order Module.symvers Module.markers
	@echo ">> Building ADS1115 kernel module..."
	@make -C $(KDIR) M=$(PWD) modules
	@echo ">> Installing ADS1115 module..."
	@sudo mkdir -p /lib/modules/$(shell uname -r)/kernel/drivers/i2c/
	@sudo cp driver_ads1115.ko /lib/modules/$(shell uname -r)/kernel/drivers/i2c/
	@sudo depmod
	@echo ">> Module installed successfully."

# Remove the kernel module from the system
uninstall_driver:
	@echo ">> Uninstalling ADS1115 module..."
	@if lsmod | grep -q driver_ads1115; then \
		sudo rmmod driver_ads1115; \
	else \
		echo "Module not loaded, skipping rmmod."; \
	fi
	@sudo depmod
	@echo ">> Module uninstalled successfully."

# Compile and install the Device Tree Overlay
create_dvt:
	@echo ">> Compiling ADS1115 overlay..."
	@dtc -@ -I dts -O dtb -o ads1115.dtbo ads1115-overlay.dts
	@sudo cp ads1115.dtbo /boot/overlays/
	@echo ">> Overlay compiled and copied."

# Enable the overlay in /boot/config.txt if not already present
enable_dvt:
	@echo ">> Enabling overlay in /boot/config.txt..."
	@if ! grep -Fxq "dtoverlay=ads1115" /boot/config.txt; then \
		echo "dtoverlay=ads1115" | sudo tee -a /boot/config.txt > /dev/null; \
		echo "Overlay entry added."; \
	else \
		echo "Overlay already enabled."; \
	fi

# Remove the overlay file and its entry in config.txt
remove_dvt:
	@echo ">> Removing ADS1115 overlay..."
	@sudo rm -f /boot/overlays/ads1115.dtbo
	@sudo sed -i '/^dtoverlay=ads1115/d' /boot/config.txt
	@echo ">> Overlay removed."

# Reboot the system (optional step after overlay change)
end_reboot:
	@echo ">> Rebooting system..."
	@sudo reboot

# Run both create_overlay and enable_overlay
test:
	@echo ">> [TEST] Creating and enabling overlay..."
	@$(MAKE) create_overlay
	@$(MAKE) enable_overlay

# Display the help message
help:
	@echo ">>> ADS1115 Driver Usage <<<"
	@echo "1. To build and install the ADS1115 kernel module:"
	@echo "   make build_and_install"
	@echo "2. To remove the ADS1115 module:"
	@echo "   make uninstall_drive"
	@echo "3. To compile and install the device tree overlay:"
	@echo "   make create_overlay"
	@echo "4. To enable the overlay in /boot/config.txt:"
	@echo "   make enable_overlay"
	@echo "5. To remove the overlay from /boot/config.txt:"
	@echo "   make remove_overlay"
	@echo "6. To reboot the system after changes:"
	@echo "   make end_reboot"
	@echo "7. To test overlay creation and enablement:"
	@echo "   make test"
	@echo "For more information, check the driver source code and documentation."
