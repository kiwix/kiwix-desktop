function init() {
    new QWebChannel(qt.webChannelTransport, function(channel) {
      settingsManager = channel.objects.settingsManager;
      app = new Vue({
        el: "#settings",
        data: {
          settingsManager: settingsManager,
          kiwixServerPort: settingsManager.kiwixServerPort,
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
            }
        }
      });
    });
}