Ext.define('Superfastmatch.view.SearchForm', {
    requires: ['Ext.ux.statusbar.ValidationStatus', 'Ext.ux.statusbar.StatusBar'],
    extend: 'Ext.form.Panel',
    alias: 'widget.searchform',

    layout: {
        type: 'fit'
    },
    
    itemId: 'SearchForm',
    bodyPadding: 10,
    title: 'Search',
    tpl: Ext.create('Ext.XTemplate','<pre class="wrap">{text}</pre>'),
    overflowY: 'auto',

    config: {
        showDocked: true,   
    },

    buildItems: function(){
        return {
            items: [Ext.create('Ext.form.field.TextArea',{
                itemId: 'SearchText',
                name: 'text',
                labelAlign: 'top',
                allowBlank: false,
                blankText: 'Please paste some text!',
                emptyText: 'Paste your search text here.',
                anchor: '100%'                
            })]
        };
    },

    constructor : function(config){
       this.initConfig(config);
       this.callParent(arguments);
    },

    initComponent: function() {
        var me = this;
        Ext.applyIf(me,me.buildItems());
        me.callParent(arguments);
        if (me.config.showDocked){
            me.addDocked([Ext.create('Ext.ux.statusbar.StatusBar', {
                itemId: 'SearchStatus',
                dock: 'bottom',
                defaultText: 'Ready',
                items: ['->', {
                    text: 'Search',
                    itemId: 'SearchButton'
                }],
                plugins: Ext.create('Ext.ux.statusbar.ValidationStatus', {
                    form: me.getId()
                })
            })]);
            me.down('#SearchButton').on('click',me.search,me);
            me.on('render',function(panel) {
                panel.body.on('click',me.unlockText,panel);
            });
        }
        me.addEvents('searching','results');
    },
    
    search: function(){
        var me=this,
            status=me.down('#SearchStatus'),
            form=me.getForm();
        if (form.isValid()){
            status.showBusy('Searching...');
            me.getEl().mask();
            var search=Ext.create('Superfastmatch.model.Search',form.getFieldValues());
            me.lockText(search.get('text'));
            search.save({
                success: me.results,
                scope: me
            });
            me.fireEvent('searching');
        }  
    },    
    
    results: function(record,operation){
        var me=this,
            status=me.down('#SearchStatus');
        status.setStatus({
            text: 'Search in '+record.get("responseTime"),
            iconCls: ''
        });
        me.getEl().unmask();
        me.fireEvent('results',record,operation);
    },
    
    lockText: function(text){
        var me=this,
            searchText=me.down('#SearchText');
        searchText.hide();
        me.enable();
        me.update({text:text});
    },
    
    unlockText: function(){
        var me=this,
            searchText=me.down('#SearchText');
        if (searchText.isHidden()){
            me.update();
            searchText.show();   
        }
    },
    
    highlightChange: function(eventArgs){
        var me=this,
            searchText=me.down('#SearchText'),
            textEl=me.body.dom,
            text=RegExp.escape((textEl.textContent|| textEl.innerText).substr(eventArgs.start,eventArgs.length)).trim();
        if (eventArgs.action==='enter'){
            highlightText(textEl,text,'highlight',true);
            Ext.get(Ext.query('span',textEl)).scrollIntoView(textEl);            
        }else if(eventArgs.action==='leave'){
            removeHighlighting('highlight',textEl);            
        }
    }
});