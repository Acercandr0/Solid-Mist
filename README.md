# Solid Mist 🪟
Windhawk theme and some tweaks for Windows 11 25H2.

![explorer](https://raw.githubusercontent.com/Acercandr0/Solid-Mist/main/Previews/explorer.png)

![start+action](https://raw.githubusercontent.com/Acercandr0/Solid-Mist/main/Previews/start+action.png)

![start+notification](https://raw.githubusercontent.com/Acercandr0/Solid-Mist/main/Previews/start+notification.png)

![lock](https://raw.githubusercontent.com/Acercandr0/Solid-Mist/main/Previews/lock.png)

---

## 🚀 AUTOMATIC INSTALLATION
The most efficient way to deploy the **Solid Mist** configuration. This method uses a PowerShell script to automate the entire setup process.

### Important Requirements:
* **Administrator Privileges:** You must run the terminal as an Administrator to allow software installation and file movement.
* **Standard Version Only:** This script is specifically designed for the **installed version** of Windhawk. It is **not compatible with Windhawk Portable**.

### How to use it:
Open **PowerShell** as Administrator and paste the following command:

`powershell -c "irm https://raw.githubusercontent.com/Acercandr0/Solid-Mist/main/Install-SolidMist.ps1 | iex"`

*The script will automatically detect if Windhawk is missing, install it via WinGet, download the latest Solid Mist configuration files, and move them to your system's Profiles folder.*

---

## 🛠️ MANUAL INSTALLATION
If you prefer a modular setup or are using the portable version, you can apply the tweaks individually.

### 1. Translucent Windows
[YAML FILE](./YAML/Translucent%20Windows.yaml)

### 2. Windows 11 File Explorer Styler
[YAML FILE](./YAML/Windows%2011%20File%20Explorer%20Styler.yaml)

### 3. Windows 11 Start Menu Styler
[YAML FILE](./YAML/Windows%2011%20Start%20Menu%20Styler.yaml)
> **📝 Optional:** To apply the lock screen style, add `LockApp.exe` to the custom process list in the Windows 11 Start Menu Styler Advanced Tab and the Process inclusion list on Windhawk Settings > Advanced Settings.

### 4. Windows 11 Taskbar Styler
[YAML FILE](./YAML/Windows%2011%20Taskbar%20Styler.yaml)
> **📝 Recommendation:** Turn Off Search on Taskbar from Settings > Personalization > Taskbar.

### 5. Taskbar ContentPresenter Injector
No config required, just install.

### 6. Windows 11 Notification Center Styler
[YAML FILE](./YAML/Windows%2011%20Notification%20Center%20Styler.yaml)
> **📝 Optional:** To apply this styling to the input and clipboard window, add `TextInputHost.exe` to the custom process list in the Windows 11 Notification Center Styler Advanced Tab and the Process inclusion list on Windhawk Settings > Advanced Settings.

### 7. Taskbar Clock Customization
[YAML FILE](./YAML/Taskbar%20Clock%20Customization.yaml)

### 8. Taskbar Tray Icon Spacing
[YAML FILE](./YAML/Taskbar%20Tray%20Icon%20Spacing.yaml)

### 9. Taskbar height and icon size
[YAML FILE](./YAML/Taskbar%20height%20and%20icon%20size.yaml)

### 10. Customize Windows notifications placement
No config required; set to "top" on the Settings tab.

### 11. Taskbar on top for Windows 11
No config required; use the Settings tab.

### 🖱️ Cursor Recommendation
[Minimalistic V3 cursor](https://www.deviantart.com/skyeo84/art/Minimalistic-V3-cursor-909469275)

---

## 🧉 Made with cariño by [@Acercandr0](https://github.com/Acercandr0)
Enjoy it. Fork it. Remix it. Make it yours.