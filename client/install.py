import os
import sqlite3
from os.path import join
from shutil import copy, rmtree, copytree
import sys
if sys.version_info < (3, 3, 0):
    FileNotFoundError = OSError

script_path = os.path.dirname(os.path.abspath(__file__))

def _get_pkg_path():
    args = sys.argv[1:]
    if "--package-path" in args:
        pkg_path = args[args.index("--package-path") +1]
    else:
        pkg_path = join(script_path, "pkg")
    return pkg_path

def package(bits="x64"):
    pkg_path = _get_pkg_path()
    print("Creating package at: %s" % pkg_path)
    service_path = join(script_path, "service")
    shared_path = join(script_path, "shared")
    docloudext_path = join(script_path, "docloudext")

    try:
        rmtree(pkg_path)
    except FileNotFoundError:
        pass
    os.mkdir(pkg_path)
    copy("install.py", pkg_path)
    copy(join(shared_path, "sqlite3", "sqlite3.dll"), pkg_path)
    copy(join(shared_path, "schema.sql"), pkg_path)
    for file in os.listdir(join(shared_path, bits)):
        copy(join(shared_path, bits, file), pkg_path)
    copy(join(docloudext_path, "docloudext.dll"), pkg_path)
    copy(join(service_path, "docloud-svc.exe"), pkg_path)

    print("Package created at: %s" % pkg_path)
    return pkg_path

def install(pkg_path):
    args = sys.argv[1:]
    install_path = " ".join(sys.argv[sys.argv.index("install")+1:])
    if not install_path:
        install_path = "install"
    install_path = os.path.abspath(install_path)
    reinstall = "--reinstall" in args or "-r" in args

    if reinstall:
        uninstall(install_path)
    print("Installing package from: %s" % pkg_path)
    previous_install_path = _get_registry_install_path()
    if previous_install_path is not None:
        print("Docloud is already installed!")
    if os.path.exists(install_path):
        print("Path already exists: %s! (use -r to reinstall)" % install_path)
        return

    _set_registry_install_path(install_path)
    print("Installing in: %s" % install_path)
    copytree(pkg_path, install_path)
    db_path = join(install_path, "db.sqlite")
    schema_path = join(pkg_path, "schema.sql") if os.path.exists(join(pkg_path, "schema.sql")) else join(pkg_path, "shared", "schema.sql")
    conn = sqlite3.connect(db_path)
    with open(schema_path) as schema_f:
        conn.executescript(schema_f.read())

    os.system("regsvr32 %s" % join(install_path, "docloudext.dll"))

    print("Starting service:")
    os.system(join(install_path, "docloud-svc.exe"))

def _get_registry_install_path():
    import winreg
    docloud_key = winreg.CreateKey(winreg.HKEY_LOCAL_MACHINE, "Software\\Docloud\\Docloud")
    try:
        install_path, no = winreg.QueryValueEx(docloud_key, "install_path")
        return install_path
    except FileNotFoundError:
        return None

def _set_registry_install_path(install_path):
    import winreg
    docloud_key = winreg.CreateKey(winreg.HKEY_LOCAL_MACHINE, "Software\\Docloud\\Docloud")
    winreg.SetValueEx(docloud_key, "install_path", 0, winreg.REG_SZ, install_path)

def _delete_registry_keys():
    import winreg
    docloud_key = winreg.CreateKey(winreg.HKEY_LOCAL_MACHINE, "Software\\Docloud\\Docloud")
    winreg.DeleteKey(docloud_key, "")

def uninstall(new_path=None):
    print("Uninstalling...")
    install_path = _get_registry_install_path()
    if install_path is not None:
        print("Removing installation at: %s" % install_path)
        docloudext_dll = join(install_path, "docloudext.dll")
        if os.path.exists(docloudext_dll):
            print("Unloading dll %s" % docloudext_dll)
            os.system("regsvr32 /u %s" % docloudext_dll)
        os.system("taskkill /f /im explorer.exe")
        if os.path.exists(install_path) and os.path.isdir(install_path):
            rmtree(install_path)
        os.system("explorer.exe")
    else:
        print("No installation found!")
    if new_path is not None:
        try:
            rmtree(new_path)
        except FileNotFoundError:
            pass
    _delete_registry_keys()

args = sys.argv[1:]
if "uninstall" in args:
    uninstall()
if "package" in args:
    pkg_path = package()
    if "install" in args:
        install(pkg_path)
elif "install" in args:
    install(_get_pkg_path())
elif "uninstall" not in args:
    print("Options are: package, install, uninstall, --killexplorer (-k), --reinstall (-r) --package-path")