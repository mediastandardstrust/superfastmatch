Ext.define('Superfastmatch.controller.Status', {
    extend: 'Ext.app.Controller',
    views: ['StatusPanel'],
    refs: [
      {
        ref: 'slots',
        selector: '#SlotGrid'
      },
      {
        ref: 'stats',
        selector: '#StatsGrid'
      },
      {
        ref: 'conf',
        selector: '#ConfigurationGrid'
      },
      {
        ref: 'dbs',
        selector: '#DatabaseGrid'
      }
    ],
    
    init: function() {
      this.control({
        '#StatusPanel':{
          activate: this.loadData
        }
      });
    },
   
    loadData: function(){
      var me=this;
      Ext.Ajax.request({
          url: '/status/',
          success: function(response){
            var json=Ext.decode(response.responseText);
            console.log(me.getConf());
            me.getSlots().getStore().loadRawData(json);
            me.getConf().setSource(json.config);
            me.getStats().setSource(json.stats);
            me.getDbs().getStore().loadRawData(json);
          }
      });
    }
});
