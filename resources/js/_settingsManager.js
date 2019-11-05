function init() {
    new QWebChannel(qt.webChannelTransport, function(channel) {
      settingsManager = channel.objects.settingsManager;
      app = new Vue({
        el: "#settings",
        data: {
          settingsManager: settingsManager,
          kiwixServerPort: settingsManager.kiwixServerPort,
          zoomFactor: Math.floor(settingsManager.zoomFactor * 100),
        },
        methods: {
            setPort : function() {
                // regex for valid port
                if (/^([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$/.test(this.kiwixServerPort))
                {
                    settingsManager.setKiwixServerPort(this.kiwixServerPort);
                } else {
                    alert("invalid port");
                    this.kiwixServerPort = settingsManager.kiwixServerPort;
                }
            },
            setZoomFactor : function() {
                this.zoomFactor = (this.zoomFactor < 30) ? 30 : this.zoomFactor;
                this.zoomFactor = (this.zoomFactor > 500) ? 500 : this.zoomFactor;
                settingsManager.setZoomFactor(this.zoomFactor / 100);
            }
        }
      });
    });
}