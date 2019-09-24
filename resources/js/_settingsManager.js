function init() {
    new QWebChannel(qt.webChannelTransport, function(channel) {
      settingsManager = channel.objects.settingsManager;
      app = new Vue({
        el: "#settings",
        data: {
          settingsManager: settingsManager,
        },
        methods: {
        }
      });
    });
}