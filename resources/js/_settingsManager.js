function onDownloadDirChanged (downloadDir) {
    app.downloadDir = downloadDir;
}

function onSettingsChecked (valid) {
    if (!valid) {
        alert("Invalid download path");
        app.downloadDir = settingsManager.downloadDir;
        return;
    }
    settingsManager.setKiwixServerPort(app.kiwixServerPort);
    app.zoomFactor = (app.zoomFactor < 30) ? 30 : app.zoomFactor;
    app.zoomFactor = (app.zoomFactor > 500) ? 500 : app.zoomFactor;
    settingsManager.setZoomFactor(app.zoomFactor / 100);
    settingsManager.setDownloadDir(app.downloadDir);
}

function validPort (port) {
    return /^([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$/.test(port);
}

function validDownloadDir (dir) {
    settingsManager.validDownloadDir(dir);
}

function setTranslations(translations) {
    app.translations = createDict(TRANSLATION_KEYS, translations);
}

const TRANSLATION_KEYS = ["settings",
                          "apply",
                          "port-for-local-kiwix-server-setting",
                          "zoom-level-setting",
                          "download-directory-setting",
                          "reset",
                          "browse"];

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
          translations:{}
        },
        methods: {
            gt : function(key) {
                return this.translations[key];
            },
            saveSettings : function() {
                if (!validPort(this.kiwixServerPort)) {
                    alert("Invalid port");
                    this.kiwixServerPort = settingsManager.kiwixServerPort;
                    return;
                }
                validDownloadDir(this.downloadDir);
            },
            resetDownloadDir : function() {
                settingsManager.resetDownloadDir();
            },
            browseDownloadDir : function() {
                settingsManager.browseDownloadDir();
            }
        }
      });
      settingsManager.downloadDirChanged.connect(onDownloadDirChanged)
      settingsManager.settingsChecked.connect(onSettingsChecked)
      settingsManager.getTranslations(TRANSLATION_KEYS, setTranslations);
    });
}