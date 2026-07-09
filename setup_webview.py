import urllib.request
import zipfile
import os
import shutil

vendor_dir = "vendor"
webview2_dir = os.path.join(vendor_dir, "webview2")

os.makedirs(vendor_dir, exist_ok=True)
os.makedirs(webview2_dir, exist_ok=True)

# 2. Download WebView2 NuGet package
print("Downloading Microsoft.Web.WebView2 NuGet package...")
nuget_url = "https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2/1.0.2592.51"
zip_path = "webview2.zip"
urllib.request.urlretrieve(nuget_url, zip_path)

# 3. Extract necessary files
print("Extracting WebView2 SDK...")
with zipfile.ZipFile(zip_path, 'r') as zip_ref:
    zip_ref.extractall("webview2_extracted")

# 4. Copy headers and DLL/Lib
include_src = os.path.join("webview2_extracted", "build", "native", "include")
x64_src = os.path.join("webview2_extracted", "build", "native", "x64")

shutil.copytree(include_src, os.path.join(webview2_dir, "include"), dirs_exist_ok=True)
shutil.copytree(x64_src, os.path.join(webview2_dir, "x64"), dirs_exist_ok=True)

# Cleanup
os.remove(zip_path)
shutil.rmtree("webview2_extracted")

print("WebView2 SDK Setup Complete!")
