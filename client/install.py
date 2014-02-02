#!/usr/bin/python3
import sys
if sys.version_info.major < 3:
    sys.stdout.write("USE PYTHON 3!!!!!!!!\n")
    sys.exit()
import os
import sqlite3
from os.path import join, splitext
from shutil import copy, rmtree, copytree

import time
import subprocess
try:
    import winreg
except ImportError:
    winreg = None

if sys.version_info < (3, 3, 0):
    FileNotFoundError = OSError

script_path = os.path.dirname(os.path.abspath(__file__))

def _spawn(process, *args):
    subprocess.Popen([process] + list(args))

def _get_pkg_path(default=None):
    args = sys.argv[1:]
    if "--package-path" in args:
        pkg_path = args[args.index("--package-path") +1]
    else:
        pkg_path = join(script_path, default if default is not None else "pkg")
    return pkg_path

def package(bits="x64"):
    pkg_path = _get_pkg_path()
    tmppkg_path = os.path.join(pkg_path, "tmppkg")
    print("Creating package at: %s" % pkg_path)
    service_path = join(script_path, "service")
    shared_path = join(script_path, "shared")
    settings_path = join(script_path, "settings")
    docloudext_path = join(script_path, "docloudext")

    try:
        rmtree(pkg_path)
    except FileNotFoundError:
        pass
    os.mkdir(pkg_path)
    os.mkdir(tmppkg_path)
    copy("install.py", pkg_path)
    copy(join(shared_path, "sqlite3", "sqlite3.dll"), tmppkg_path)
    copy(join(shared_path, "schema.sql"), tmppkg_path)

    if bits == "x64":
        dlls = join(shared_path, "libwin64", "gtk3")
    else:
        dlls = join(shared_path, "libwin32", "gtk3")

    if os.path.exists(dlls) and os.path.isdir(dlls):
        for root, dirs, files in os.walk(dlls):
            for file in files:
                if splitext(file)[1] == ".dll":
                    copy(join(dlls, root, file), join(tmppkg_path, file))

    copy(join(docloudext_path, "docloudext.dll"), tmppkg_path)
    copy(join(service_path, "docloud-svc.exe"), tmppkg_path)

    copy(join(settings_path, "docloud-settings.exe"), tmppkg_path)
    copy(join(settings_path, "main.ui"), tmppkg_path)

    _zipdir(tmppkg_path, os.path.join(pkg_path, "pkg.zip"))
    rmtree(tmppkg_path)
    print("Package created at: %s" % pkg_path)
    return pkg_path

import zipfile
def _zipdir(directory, zip_path):
    with zipfile.ZipFile(zip_path, 'w') as zipf:
        for root, dirs, files in os.walk(directory):
            for file in files:
                zipf.write(join(root, file), file)

def _unzip(zip_path, destination):
    with zipfile.ZipFile(zip_path, "r") as zipf:
        zipf.extractall(destination)

def install(pkg_path):
    args = sys.argv[1:]
    install_path = " ".join(sys.argv[sys.argv.index("install")+1:])
    if not install_path:
        install_path = "install"
    install_path = os.path.abspath(install_path)
    reinstall = "--reinstall" in args or "-r" in args

    if reinstall:
        if not uninstall(install_path):
            print("Uninstall failed! Not running re-installation!")
    print("Installing package from: %s" % pkg_path)
    previous_install_path = _get_registry_install_path()
    if previous_install_path is not None:
        print("Docloud is already installed! No install needed (use -r to reinstall)")
        return
    if os.path.exists(install_path):
        print("Path already exists: %s! No install run (use -r to reinstall)" % install_path)
        return

    _set_registry_install_path(install_path)
    print("Installing in: %s" % install_path)
    tmppkg_path = os.path.join(pkg_path, "tmppkg")
    _unzip(os.path.join(pkg_path, "pkg.zip"), tmppkg_path)

    db_path = join(tmppkg_path, "db.sqlite")
    schema_path = join(tmppkg_path, "schema.sql") \
        if os.path.exists(join(tmppkg_path, "schema.sql")) else join(tmppkg_path, "shared", "schema.sql")

    print("Installing db-schema")
    try:
        conn = sqlite3.connect(db_path)
    except Exception as e:
        print("Could not connect to dababase: %s" % e)
        print("Install stopped")
        return

    try:
        with open(schema_path) as schema_f:
            conn.executescript(schema_f.read())
    except Exception as e:
        print("Could not create schema in db: %s" % e)
        print("Install stopped")
        conn.close()
        return
    conn.close()
    try:
        copytree(tmppkg_path, install_path)
    except OSError as e:
        print("Error copying files: %s" % e)
        print("Install stopped")
        return
    try:
        rmtree(tmppkg_path)
    except OSError as e:
        print("Error removing files: %s" % e)

    print("Loading dll: %s" % join(install_path, "docloudext.dll"))
    os.system("regsvr32 /s %s" % join(install_path, "docloudext.dll"))

    print("Starting service:")
    os.system(join(install_path, "docloud-svc.exe"))

def _get_registry_install_path():
    docloud_key = winreg.CreateKey(winreg.HKEY_LOCAL_MACHINE, "Software\\Docloud\\Docloud")
    try:
        install_path, no = winreg.QueryValueEx(docloud_key, "install_path")
        return install_path
    except FileNotFoundError:
        return None

def _set_registry_install_path(install_path):
    docloud_key = winreg.CreateKey(winreg.HKEY_LOCAL_MACHINE, "Software\\Docloud\\Docloud")
    winreg.SetValueEx(docloud_key, "install_path", 0, winreg.REG_SZ, install_path)

def _delete_registry_keys():
    docloud_key = winreg.CreateKey(winreg.HKEY_LOCAL_MACHINE, "Software\\Docloud\\Docloud")
    winreg.DeleteKey(docloud_key, "")

def uninstall(new_path=None):
    success = True
    print("Uninstalling...")
    install_path = _get_registry_install_path()
    if install_path is not None:
        print("Removing installation at: %s" % install_path)
        docloudext_dll = join(install_path, "docloudext.dll")
        if os.path.exists(docloudext_dll):
            print("Unloading dll %s" % docloudext_dll)
            os.system("regsvr32 /s /u %s" % docloudext_dll)
        os.system("taskkill /f /im explorer.exe")
        os.system("taskkill /f /im docloud-svc.exe")
        time.sleep(0.5)

        if os.path.exists(install_path) and os.path.isdir(install_path):
            try:
                rmtree(install_path)
            except OSError as e:
                print("Error removing files: %s" % e)
                success = False
        _spawn("explorer")
        time.sleep(0.5)
    else:
        print("No installation found!")
    if new_path is not None:
        try:
            rmtree(new_path)
        except FileNotFoundError:
            pass
        except OSError as e:
            print("Error removing files: %s" % e)
            success = False
    _delete_registry_keys()
    return success

args = sys.argv[1:]
if "uninstall" in args:
    uninstall()
if "package" in args:
    x86 = "--target-x86" in args
    pkg_path = package("x86" if x86 else"x64")
    if "install" in args:
        install(pkg_path)
elif "install" in args:
    install(_get_pkg_path("."))
elif "uninstall" not in args:
    print("Options are: package, install, uninstall, --target-x64, --target-x86, --killexplorer (-k), --reinstall (-r) --package-path")
