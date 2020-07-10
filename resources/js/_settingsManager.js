function onDownloadDirChanged (downloadDir) {
    app.downloadDir = downloadDir;
}

function validPort (port) {
    return /^([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$/.test(port);
}

function setTranslations(translations) {
    app.translations = createDict(TRANSLATION_KEYS, translations);
}

const TRANSLATION_KEYS = ["settings",
                          "port-for-local-kiwix-server-setting",
                          "zoom-level-setting",
                          "download-directory-setting",
                          "reset",
                          "browse",
                          "invalid-port"];

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
            setPort : function() {
                if (!validPort(this.kiwixServerPort)) {
                    alert(this.gt("invalid-port"));
                    this.kiwixServerPort = settingsManager.kiwixServerPort;
                    return;
                }
                settingsManager.setKiwixServerPort(this.kiwixServerPort);
            },
            setZoomFactor : function() {
                this.zoomFactor = (this.zoomFactor < 30) ? 30 : this.zoomFactor;
                this.zoomFactor = (this.zoomFactor > 500) ? 500 : this.zoomFactor;
                settingsManager.setZoomFactor(this.zoomFactor / 100);
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
      settingsManager.getTranslations(TRANSLATION_KEYS, setTranslations);
    });
}