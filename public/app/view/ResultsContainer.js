Ext.define('Superfastmatch.view.ResultsContainer', {
    extend: 'Ext.container.Container',
    alias: 'widget.resultscontainer',
    requires: ['Superfastmatch.store.Fragments'],

    itemId: 'Results',
    layout: {
       align: 'stretch',
       type: 'vbox'
    },

    buildItems: function(){
        return {
            items: [
                {
                    xtype: 'gridpanel',
                    itemId: 'Documents',
                    title: 'Matching Documents',
                    forceFit: true,
                    flex: 1,
                    viewConfig: {
                        id: '',
                        itemId: 'DocumentsView'
                    },
                    columns: [],
                    selModel: Ext.create('Ext.selection.RowModel', {
                        allowDeselect: true,
                        mode: 'SIMPLE'
                    })
                },
                {
                    xtype: 'gridpanel',
                    itemId: 'Fragments',
                    title: 'Fragments',
                    forceFit: true,
                    flex: 2,
                    columns: [
                        {
                            xtype: 'gridcolumn',
                            id: '',
                            itemId: 'FragmentTextColumn',
                            dataIndex: 'text',
                            flex: 8,
                            text: 'Text'
                        },
                        {
                            xtype: 'numbercolumn',
                            dataIndex: 'length',
                            flex: 1,
                            text: 'Length',
                            format: '0,000'
                        },
                        {
                            xtype: 'numbercolumn',
                            dataIndex: 'count',
                            flex: 1,
                            text: 'Count',
                            format: '0,000'
                        }
                    ],
                    viewConfig: {
                        itemId: 'FragmentView'
                    }
                }
            ]
        };
    },
    
    allFragments: '',
    selectedFragments: '',

    initComponent: function() {
        var me = this;
        Ext.applyIf(me,me.buildItems());
        me.callParent(arguments);
        me.allFragments=Ext.create('Superfastmatch.store.Fragments');
        me.selectedFragments=Ext.create('Superfastmatch.store.Fragments');
        me.down('#FragmentTextColumn').renderer=me.columnWrap;
        me.down('#Documents').on({
            reconfigure:     me.buildFragments,
            selectionchange: me.buildFragments,
            scope:           me
        });
        me.down('#Fragments').on({
           itemmouseenter: me.highlightFragment,
           itemmouseleave: me.unHighlightFragment,
           scope:          me
        });
        me.addEvents('highlightchange');
    },
    
    loading: function(){
        this.setLoading(true);
    },
    
    loadMatches: function(search){
        var me=this;
            documents=me.down('#Documents'),
            fragments=me.down('#Fragments');
        documents.reconfigure(search.documents(),search.documents().model.getColumns());
        me.enable();
        me.setLoading(false);
    },

    highlightFragment: function(view,record){
        this.fireEvent('highlightchange',{action:'enter',start: record.get('from'),length: record.get('length')});
    },
    
    unHighlightFragment: function(view,record){
        this.fireEvent('highlightchange',{action:'leave',start: record.get('from'),length: record.get('length')});
    },

    buildFragments: function(){
        var me=this,
            documents=me.down('#Documents'),
            fragments=me.down('#Fragments'),
            selected=documents.getSelectionModel().getSelection();
        fragments.suspendEvents();
        if (selected.length==0){
            me.allFragments.suspendEvents();
            me.allFragments.loadDocuments(documents.getStore().data.items);
            me.allFragments.resumeEvents();
            fragments.reconfigure(me.allFragments);
            fragments.setTitle('Fragments (All Documents)');
        }else{
            me.selectedFragments.suspendEvents();
            me.selectedFragments.loadDocuments(selected);
            me.selectedFragments.resumeEvents();
            fragments.reconfigure(me.selectedFragments);
            fragments.setTitle('Fragments ('+selected.length+' Document'+((selected.length>1)?'s':'')+')');
        }
        fragments.resumeEvents();
    },

    columnWrap: function(val){
        return '<div style="white-space:normal !important;"><pre class="wrap">'+ val +'</pre></div>';
    },
    
    
});