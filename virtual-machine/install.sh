#!/bin/bash

# Default parameters
OS_IMG_FILE="ubuntu-20.04.6-desktop-amd64.iso"
OS_TYPE="Ubuntu_64"
OS_IMG_URL="https://releases.ubuntu.com/focal/${OS_IMG_FILE}"
VM_IMG_NAME="convoy-v2x"
VM_DIR_DEFAULT="${HOME}/vm"
VM_CPU_RAM_MB="4096"
VM_GPU_RAM_MB="128"
VM_CPU_COUNT="4"
VM_IMG_DISK_MB="30000"
VM_LOGIN_USER="convoy-v2x"
VM_LOGIN_PASS="convoy-v2x"

# Read destination path for the virtual machine files
read -p "Enter the absolute destination path for the virtual machine files [${VM_DIR_DEFAULT}] : " dest_path_input
VM_DIR=${dest_folder_input:-${VM_DIR_DEFAULT}}

# Create destination folder if not already present and move into it
mkdir -p ${VM_DIR}
if [ $? -ne 0 ] ; then
    echo "Error observed while creating the destination folder, exiting"
    exit 1
fi
pushd ${VM_DIR} >> /dev/null

echo "Downloading os-installer image for the virtual machine to ${VM_DIR} ..."
wget ${OS_IMG_URL}

echo ""
echo "Creating and setting up virtual machine ${VM_IMG_NAME} ..."
VBoxManage createvm --name "${VM_IMG_NAME}" --ostype "${OS_TYPE}" --register --basefolder "${VM_DIR}"
VBoxManage modifyvm "${VM_IMG_NAME}" --description "Virtual machine for ConvoyV2X simulation"
VBoxManage modifyvm "${VM_IMG_NAME}" --ioapic on                     
VBoxManage modifyvm "${VM_IMG_NAME}" --memory "${VM_CPU_RAM_MB}" --vram "${VM_GPU_RAM_MB}"
VBoxManage modifyvm "${VM_IMG_NAME}" --nic1 nat
VBoxManage modifyvm "${VM_IMG_NAME}" --cpus "${VM_CPU_COUNT}"
VBoxManage createmedium disk --filename "$(pwd)/${VM_IMG_NAME}/${VM_IMG_NAME}.vdi" --size "${VM_IMG_DISK_MB}" --format VDI
VBoxManage storagectl "${VM_IMG_NAME}" --name "SATA Controller" --add sata --controller IntelAhci       
VBoxManage storageattach "${VM_IMG_NAME}" --storagectl "SATA Controller" --port 0 --device 0 --type hdd --medium "$(pwd)/${VM_IMG_NAME}/${VM_IMG_NAME}.vdi"

echo ""
echo "-----------------------------------------------------------------------"
echo "The virtual machine will now be started, a new desktop window will be opened, and the OS-installation triggered"
echo "Following a successfull installation the virtual machine will be automatically shutdown"
echo "A default user account with root privilegs username: ${VM_LOGIN_USER} password: ${VM_LOGIN_USER} will be created during the installation"
echo "Please wait until the entire installation sequence is complete, the desktop window of the virtual machine vanishes, and until this script ends"
echo "-----------------------------------------------------------------------"
echo ""
read -p "Press enter to continue with the installation : " install_signal_input

VBoxManage unattended install "${VM_IMG_NAME}" --iso "${OS_IMG_FILE}" --user "${VM_LOGIN_USER}" --password "${VM_LOGIN_USER}" --start-vm gui --post-install-command poweroff
echo "Installing the OS..."

while :
do
    sleep 10s
    vm_status=$(VBoxManage showvminfo "${VM_IMG_NAME}" --machinereadable | grep -i "VMState=")
    if [[ "$vm_status" =~ "poweroff" ]] ; then
        echo "$(date) - OS installation is complete"
        break
    else
        echo "$(date) - OS installation is running"
    fi
done

echo ""
echo "Removing os-installer image and exiting script"
rm -rf ${OS_IMG_FILE}
echo ""
popd >> /dev/null