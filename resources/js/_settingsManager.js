function onDownloadDirChanged (downloadDir) {
    app.downloadDir = downloadDir;
}

function onProfileDirChanged (profileDir) {
    app.profileDir = profileDir;
}

function onDownloadDirChecked (valid) {
    if (!valid) {
        alert("Invalid download directory");
        app.downloadDir = settingsManager.downloadDir;
        return;
    }
    settingsManager.validProfileDir(app.profileDir);
}

function onProfileDirChecked (valid) {
    if (!valid) {
        alert("Invalid profile directory");
        app.profileDir = settingsManager.profileDir;
        return;
    }
    settingsManager.moveProfileFiles(app.profileDir);
    setAllSettings();
}

function setAllSettings() {
    settingsManager.setKiwixServerPort(app.kiwixServerPort);
    app.zoomFactor = (app.zoomFactor < 30) ? 30 : app.zoomFactor;
    app.zoomFactor = (app.zoomFactor > 500) ? 500 : app.zoomFactor;
    settingsManager.setZoomFactor(app.zoomFactor / 100);
    settingsManager.setDownloadDir(app.downloadDir);
    settingsManager.setProfileDir(app.profileDir);
}

function validPort (port) {
    return /^([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$/.test(port);
}

function init() {
    new QWebChannel(qt.webChannelTransport, function(channel) {
      settingsManager = channel.objects.settingsManager;
      app = new Vue({
        el: "#settings",
        data: {
          settingsManager: settingsManager,
          kiwixServerPort: settingsManager.kiwixServerPort,
          zoomFactor: Math.floor(settingsManager.zoomFactor * 100),
          downloadDir: settingsManager.downloadDir,
          profileDir: settingsManager.profileDir,
        },
        methods: {
            saveSettings : function() {
                if (!validPort(this.kiwixServerPort)) {
                    alert("Invalid port");
                    this.kiwixServerPort = settingsManager.kiwixServerPort;
                    return;
                }
                settingsManager.validDownloadDir(this.downloadDir);
            },
            resetDownloadDir : function() {
                settingsManager.resetDownloadDir();
            },
            browseDownloadDir : function() {
                settingsManager.browseDownloadDir();
            },
            resetProfileDir : function() {
                settingsManager.resetProfileDir();
            },
            browseProfileDir : function() {
                settingsManager.browseProfileDir();
            }
        }
      });
      settingsManager.downloadDirChanged.connect(onDownloadDirChanged)
      settingsManager.profileDirChanged.connect(onProfileDirChanged)
      settingsManager.downloadDirChecked.connect(onDownloadDirChecked)
      settingsManager.profileDirChecked.connect(onProfileDirChecked)
    });
}