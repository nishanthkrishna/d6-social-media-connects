<?php
$options['attributes']['class']='fb-connect';
print l(t('connect with Facebook'),$login_url,$options);
if($display=='popup') {

    drupal_add_js('$(document).ready(function(){
				$(".fb-connect").click(function() { 
				var pWidth = 600;
		        var pHeight = 400;
				var pLeft = (screen.width/2)-(pWidth/2);
		        var pTop = (screen.height/2)-(pHeight/2);
				window.open (this.href,"fbConnectPopUp",\'location=0,status=0,scrollbars=0,menubar=0,resizable=1,width=600,height=400,top=\' + pTop + \',left=\' + pLeft);
				return false;
				
				} )
			});
			function FBConnectReloadPage(){
	           window.location.reload();
            }
','inline');

}

?>

