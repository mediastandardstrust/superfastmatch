Ext.define('Superfastmatch.controller.Index', {
    extend: 'Ext.app.Controller',
    views: ['IndexPanel'],
    refs: [
      {
        ref: 'index',
        selector: '#IndexPanel'
      }
    ],
    
    init: function() {
      this.control({
        '#IndexPanel':{
          activate: function(panel){panel.down('#IndexPaging').getStore().load();}
        }
      });
    }
});
