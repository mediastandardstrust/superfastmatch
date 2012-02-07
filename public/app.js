Ext.Loader.setConfig({
    enabled: true,
    disableCaching: false,
    paths: {
        'Ext': 'extjs/src',
        'Ext.ux': 'app/ux'
    }
});

Ext.Ajax.timeout = 60000; // 60 seconds

Ext.state.Manager.setProvider(
    new Ext.state.CookieProvider({
        expires: new Date(new Date().getTime()+(1000*60*60*24*365)), //1 year from now
}));

Ext.application({
    name: 'Superfastmatch',
    models: ['Fragment','Search','Document'],
    controllers: ['Searches','Documents','Compare'],
    requires: ['Superfastmatch.view.MainViewPort'],
    launch: function(){
        Ext.QuickTips.init();
        var cmp1 = Ext.create('Superfastmatch.view.MainViewPort', {
            renderTo: Ext.getBody()
        });
        cmp1.show();
    }
});
