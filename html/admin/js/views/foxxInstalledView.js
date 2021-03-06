window.FoxxInstalledView = Backbone.View.extend({
  tagName: 'li',
  className: "span3",
  template: new EJS({url: 'js/templates/foxxInstalledView.ejs'}),
  
  events: {
    'click #install': 'installFoxx'
  },
  
  initialize: function(){
    _.bindAll(this, 'render');
  },
  
  installFoxx: function(event) {
    event.stopPropagation();
    window.App.navigate("application/available/" + encodeURIComponent(this.model.get("_key")), {trigger: true});
  },
  
  render: function(){
    $(this.el).html(this.template.render(this.model));
    return $(this.el);
  }
});
