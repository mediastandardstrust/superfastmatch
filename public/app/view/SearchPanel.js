Ext.define('Superfastmatch.view.SearchPanel', {
    extend: 'Ext.panel.Panel',
    alias: 'widget.searchpanel',
    requires: [
        'Superfastmatch.view.SearchForm',
        'Superfastmatch.view.ResultsContainer'
    ],

    itemId: 'SearchPanel',
    layout: {
        align: 'stretch',
        type: 'hbox'
    },
    title: 'Search',
    
    buildItems: function(){
      return {
          items: [{
                    xtype: 'searchform',
                    flex: 1
                },{
                    xtype: 'resultscontainer',
                    flex: 1,
                    disabled: true
                }]
        }
    },
    
    initComponent: function() {
        var me = this;
        Ext.applyIf(me,me.buildItems());
        me.callParent(arguments);
    }
});