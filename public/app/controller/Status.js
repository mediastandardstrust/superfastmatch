Ext.define('Superfastmatch.controller.Status', {
    extend: 'Ext.app.Controller',
    views: ['StatusPanel'],
    refs: [
      {
        ref: 'panel',
        selector: '#StatusPanel'
      },
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
          activate: function(panel){
            this.loadData();
          }
        }
      });
    },
   
    loadProperties: function(grid,data){
      var renderers={};
      Ext.Object.each(data,function(k,v){
        if(Ext.isNumber(v)){renderers[k]=Ext.util.Format.numberRenderer("0,000");}
      });
      grid.customRenderers=renderers;
      grid.setSource(data);
    },
   
    loadData: function(){
      var me=this;
      Ext.Ajax.request({
          url: '/status/',
          success: function(response){
            var json=Ext.decode(response.responseText);
            me.getSlots().getStore().loadRawData(json);
            me.getDbs().getStore().loadRawData(json);
            me.loadProperties(me.getConf(),json.config);
            me.loadProperties(me.getStats(),json.stats);
          }
      });
      var task = Ext.create('Ext.util.DelayedTask', function() {
        if (me.getPanel().isVisible()){
          me.loadData();        
        }
      },me);
      task.delay(2000);
    }
});
