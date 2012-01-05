Ext.define('Superfastmatch.view.DocumentPanel', {
    extend: 'Ext.panel.Panel',
    alias: 'widget.documentpanel',
    requires: [
        'Superfastmatch.view.SearchForm',
        'Superfastmatch.view.ResultsContainer',
        'Superfastmatch.view.DocumentBrowser'
    ],

    itemId: 'DocumentPanel',
    layout: {
        align: 'stretch',
        type: 'hbox'
    },
    title: 'Documents',    
    buildItems: function(){
      return {
          items: [
              {
                  xtype: 'container',
                  layout: {
                      align: 'stretch',
                      type: 'vbox'
                  },
                  flex: 1,
                  items: [
                            Ext.create('Superfastmatch.view.DocumentBrowser'),  
                            Ext.create('Superfastmatch.view.SearchForm',{
                                  disabled:   true,
                                  title:      'Text',
                                  showDocked: false,
                                  flex: 2 
                            })
                ]
              },
              Ext.create('Superfastmatch.view.ResultsContainer',{
                  flex: 1                    
            }) 
          ]
        }
    },
    
    initComponent: function() {
        var me = this;
        Ext.applyIf(me,me.buildItems());
        me.callParent(arguments);
    }
});