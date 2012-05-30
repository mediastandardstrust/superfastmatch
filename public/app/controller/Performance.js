Ext.define('Superfastmatch.controller.Performance', {
    extend: 'Ext.app.Controller',
    views: ['PerformancePanel'],
    refs: [
      {
        ref: 'panel',
        selector: '#PerformancePanel'
      },
      {
        ref: 'documents',
        selector: '#PerformancePanel #DocumentGrid'
      },
      {
        ref: 'associations',
        selector: '#PerformancePanel #AssociationGrid'
      }
    ],
    
    init: function() {
      this.control({
        '#PerformancePanel':{
          activate: function(panel){
            this.loadData();
          }
        }
      });
    },
   
    loadGrid: function(grid,data){
      grid.getStore().loadRawData(data);
      grid.reconfigure(null,Ext.ModelManager.getModel(grid.getStore().model).getColumns());
    },
   
    loadData: function(){
      var me=this;
      Ext.Ajax.request({
        url: '/performance/',
        success: function(response){
          var json=Ext.decode(response.responseText);
          me.loadGrid(me.getDocuments(),json.Document);
          me.loadGrid(me.getAssociations(),json.Association);
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
