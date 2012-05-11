Ext.define('Superfastmatch.view.MainViewPort', {
    extend: 'Ext.container.Viewport',
    requires: [
        'Superfastmatch.view.SearchPanel',
        'Superfastmatch.view.DocumentPanel',
        'Superfastmatch.view.IndexPanel',
        'Superfastmatch.view.QueuePanel',
        'Superfastmatch.view.StatusPanel',
        'Ext.ux.panel.HTMLPanel',
        'Ext.ux.panel.StatefulTabPanel'
    ],

    layout: {
        type: 'fit'
    },
    
    buildItems: function(){
      return {
          items: [
            {
                xtype: 'tabpanel',
                // xtype: 'statefultab',
                id: 'Tabs',
                items: [
                    {
                        xtype: 'searchpanel'
                    },
                    {
                        xtype: 'documentpanel'
                    },
                    {
                        xtype: 'indexpanel'
                    },
                    {
                        xtype: 'queuepanel'
                    },
                    {
                        xtype: 'HTMLPanel',
                        loader: {
                            url: '/describe/'
                        },
                        title: 'Documentation'
                    },
                    {
                        xtype: 'HTMLPanel',
                        loader: {
                            url: '/performance/'
                        },
                        title: 'Performance'
                    },
                    {
                        xtype: 'statuspanel'
                    }
                ]
            }
        ]};
    },
    
    initComponent: function() {
        var me = this;
        Ext.applyIf(me,me.buildItems());
        me.callParent(arguments);
    }
});