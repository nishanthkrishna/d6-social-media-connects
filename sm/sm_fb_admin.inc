<?php

function sm_fb_settings_form() {
    $form = array();

    $form['sm_fb_facebook_app_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Facebook APP ID:'),
    '#default_value' => variable_get('sm_fb_facebook_app_id',''),
    '#size' => 100,
    '#maxlength' => 255,
    '#description' => t("Facebook APP ID:"),
    '#required' => TRUE,
    );

    $form['sm_fb_facebook_app_secret'] = array(
    '#type' => 'textfield',
    '#title' => t('Facebook APP Secret:'),
    '#default_value' => variable_get('sm_fb_facebook_app_secret',''),
    '#size' => 100,
    '#maxlength' => 255,
    '#description' => t("Facebook APP Secret:"),
    '#required' => TRUE,
    );

    $form['sm_fb_facebook_app_permissions'] = array(
    '#type' => 'textarea',
    '#title' => t('Facebook APP permissions:'),
    '#default_value' => variable_get('sm_fb_facebook_app_permissions',''),
    '#description' => t("Facebook APP permissions:"),
    '#required' => TRUE,
    );


    $form['sm_fb_facebook_display'] = array(
    '#type' => 'select',
    '#title' => t('Display mode in which to render the Dialog.'),
    '#default_value' => variable_get('sm_fb_facebook_display','page'),
    '#options' => array(
    'page' => t('page'),
    'popup' => t('popup'),
    'iframe' => t('iframe'),
    'touch' => t('touch'),
    ),
    '#description' => t('Display mode in which to render the Dialog.'),

    );
    return system_settings_form($form);
}