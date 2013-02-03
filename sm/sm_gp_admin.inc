<?php

function sm_gp_settings_form() {
    $form = array();

    $form['sm_gp_client_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Client ID:'),
    '#default_value' => variable_get('sm_gp_client_id',''),
    '#size' => 100,
    '#maxlength' => 255,
    '#description' => t("Client ID : https://code.google.com/apis/console"),
    '#required' => TRUE,
    );

    $form['sm_gp_secret'] = array(
    '#type' => 'textfield',
    '#title' => t('Client secret:'),
    '#default_value' => variable_get('sm_gp_secret',''),
    '#size' => 100,
    '#maxlength' => 255,
    '#description' => t("Client secret:"),
    '#required' => TRUE,
    );

    $form['sm_fb_developer_key'] = array(
    '#type' => 'textarea',
    '#title' => t('Developer key:'),
    '#default_value' => variable_get('sm_fb_developer_key',''),
    '#description' => t("Developer key:"),
    '#required' => TRUE,
    );

    $form['sm_fb_developer_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Developer key:'),
    '#default_value' => variable_get('sm_fb_developer_key',''),
    '#size' => 100,
    '#maxlength' => 255,
    '#description' => t("Developer key:"),

    );
    $redirect = url("gp/connect",array("absolute"=>true));

    $form['markup']=array(
    "#value"=>"<label>Redirect URL: $redirect</label>",
    "#prefix"=>"<div class='form-item'>",
    "#suffix"=>"</div>",
    );
 

    return system_settings_form($form);
}