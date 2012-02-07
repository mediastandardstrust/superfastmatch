Ext.define('Superfastmatch.controller.Compare', {
    extend: 'Ext.app.Controller',
    requires: ['Superfastmatch.view.SideBySide'],
    
    init: function() {
      this.control({
        '#Results' :{
            comparedocument: this.onDocumentCompare
        }
      });
    },

    onDocumentCompare: function(evArgs){
      Ext.create('Superfastmatch.view.SideBySide').compare(evArgs);
    }
});