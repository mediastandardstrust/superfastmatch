Ext.define('Superfastmatch.controller.Queue', {
    extend: 'Ext.app.Controller',
    stores: ['Queue'],
    views: ['QueuePanel'],
    refs: [
      {
        ref: 'queue',
        selector: '#QueuePanel'
      }
    ],
    
    init: function() {
      this.control({
        '#QueuePanel':{
          activate: function(panel){panel.getStore().load({start:1});}
        }
      });
    },
    
    onLaunch: function(){
      // this.getQueue().reconfigure(this.getQueueStore());
    }
});
