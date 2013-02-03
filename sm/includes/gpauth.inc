<?php

function sm_gp_connect() {
    global $user;
    $client=_sm_gp_init();
    if(isset($_GET['error'])) {
        drupal_set_message(t("Google connect error: %error ",array('%error'=>$_GET['error'])),'error');
        drupal_goto('user/register');
    }
    if (isset($_GET['code'])) {
       
        $client->authenticate();
        $_SESSION['google_access_token'] = $client->getAccessToken();
        $redirect = url("gp/connect",array("absolute"=>true));
        header('Location: ' . filter_var($redirect, FILTER_SANITIZE_URL));
        exit();
    }

    try  {

        if (isset($_SESSION['google_access_token'])) {
            $client->setAccessToken($_SESSION['google_access_token']);
        }
        if ( $token=$client->getAccessToken()) {
            $gp_lib_path = function_exists('libraries_get_path') ? libraries_get_path('google-api-php-client') : 'sites/all/libraries/google-api-php-client';
            $token=json_decode($token);
            $token_value=$token->access_token;
            $request="https://www.googleapis.com/oauth2/v1/userinfo?access_token={$token_value}";
            $result=drupal_http_request($request);

            if($result->code==200 && isset($result->data)) {
                $info=json_decode($result->data);
                $gp_id=$info->id;
                if($gp_id && !$user->uid) {
                    // $account=sm_gp_get_local_user($gp_id);
                    $account=sm_gp_get_local_user($gp_id);

                    if(!$account) {
                        $account= sm_gp_create_local_user($info);
                    }
                    if($account) {
                        _sm_gp_external_login($gp_id,$account);
                        drupal_goto('user');
                    }
                }
            }

        }

    }catch (Exception $e) {
        drupal_set_message($e->getMessage(),'error');
        drupal_goto('user/register');

    }

   exit();

}

function sm_gp_create_local_user($info) {

    $gpu=$info->id;
    $account = sm_gp_get_local_user($gpu);

    if(!$account && $gpu) {
        $account = sm_gp_get_local_user_by_email($info);
        _sm_gp_set_map($account, $gpu);
        sm_gp_invoke('profile_save', $info, $account, 'sm_gp');
    }
    if(!$account ) {
        $username = "$gpu@google";
        if(isset($info->username)) {
            $username=$info->username;
        } else {
            $username = "$gpu@google";
        }
        if(!isset($info->email)) {
            $info->email= "$gpu@google";
        }
        $edit['name']=$username;

        $i = 1;
        while (db_result(db_query("SELECT name FROM {users} WHERE name='%s'", $edit['name']))) {
            $i++;
            $edit['name'] = $username . '_' . $i;
        }

        $edit['roles'][DRUPAL_AUTHENTICATED_RID] = 'authenticated user';

        $defaults = array(
        'pass' => user_password(),
        'init' => $info->email,
        'status' => 1,
        );
        $defaults['mail'] = $info->email;

        $edit = array_merge($defaults, $edit);
        if ($uid = db_result(db_query("SELECT uid FROM {users} WHERE name='%s'", $edit['name']))) {
            // The desired name is taken.
            watchdog('gp_user', 'Failed to create new user %name.  That name is already in the users table.',
            array('%name' => $edit['name']),
            WATCHDOG_ERROR, l(t('view user'), 'user/' . $uid));
        }
        else {
            $account = user_save('', $edit);
            _sm_gp_set_map($account, $gpu);
            sm_gp_invoke('profile_save', $info, $account, 'sm_gp');
            watchdog('gp_user', 'New user: %name %email.',
            array('%name' => $account->name, '%email' => '<'. $account->mail .'>'),
            WATCHDOG_NOTICE, l(t('edit'), 'user/'. $account->uid .'/edit'));
        }
    }

    return $account;

}
function _sm_gp_get_uid($gpu) {
    $result = db_result(db_query("SELECT uid FROM {gp_user} WHERE gpu=%d", array(
    $gpu,
    )));

    return $result;
}

function sm_gp_get_local_user($gpu) {
    if ($uid = _sm_gp_get_uid($gpu)) {
        return user_load($uid);
    }
}

function  _sm_gp_set_map($account, $gpu) {
    if ($gpu &&
    $account->uid != 0) {
        db_query("DELETE FROM {gp_user} WHERE uid=%d OR gpu=%d", $account->uid, $gpu);
        db_query("INSERT INTO {gp_user} (uid, gpu) VALUES (%d, %d)", $account->uid, $gpu);
        watchdog('gp_user', 'Using gp_user to associate user !user with google user id %gpu.',
        array('!user' => l($account->name, 'user/' . $account->uid),
        '%gpu' => $fbu,
        ));

    }
}

function _sm_gp_external_login($gpu,$account) {
    if ($account &&
    $account->uid == $GLOBALS['user']->uid) {
        return $account;
    }
    elseif ($gpu &&
    $account &&
    $account->uid != $GLOBALS['user']->uid ) {
        $session_id = session_id();
        watchdog("gp_user", "gp_user changing user to $account->uid");
        // user_external_login() fails if already logged in, so log out first.
        if ($GLOBALS['user']->uid) {
            session_destroy();
            $GLOBALS['user'] = drupal_anonymous_user();
        }
        $valid_user = user_external_login($account);
        if ($valid_user) {
            return $account;
        }
    }
    return FALSE;
}
function _sm_gp_init() {
    $gp_lib_path = function_exists('libraries_get_path') ? libraries_get_path('google-api-php-client') : 'sites/all/libraries/google-api-php-client';
    $gp_platform = $gp_lib_path . '/src/apiClient.php';
    try {
        if (!class_exists('apiClient') && !include_once($gp_platform)) {
            $message = t('Failed to find the apiClient client libraries ');
            drupal_set_message($message, 'error');
            watchdog('sm_gp', $message);
            return NULL;
        }
        $client = new apiClient();
        $client->setApplicationName("Google Connect");
        $client->setClientId(variable_get('sm_gp_client_id',''));
        $client->setClientSecret(variable_get('sm_gp_secret',''));
        $client->setRedirectUri(url("gp/connect",array("absolute"=>true)));
        $client->setDeveloperKey(variable_get('sm_fb_developer_key',''));
        $client->setScopes(array('https://www.googleapis.com/auth/plus.me','https://www.googleapis.com/auth/userinfo.email','https://www.googleapis.com/auth/userinfo.profile'));
        return $client;

    }catch (Exception $e) {
        drupal_set_message($e->getMessage());

    }

}
function sm_gp_invoke($op, $data = NULL, $return = NULL, $hook = 'sm_gp') {
    foreach (module_implements($hook) as $name) {
        $function = $name . '_' . $hook;
        try {
            $function($op, $data, $return);
        }
        catch (Exception $e) {

        }
    }

    return $return;
}

function sm_gp_get_local_user_by_email($info) {

    if (isset($info->id) && $info->id) {
        try {
            if (isset($info->email) &&
            ($email =$info->email)) {
                return user_load(array('mail' => $email));
            }
        }
        catch (Exception $e) {
            // This can occur when user logs out of facebook in another window, then returns to our site.

            // t('Failed to get google user email.');

        }
    }
}