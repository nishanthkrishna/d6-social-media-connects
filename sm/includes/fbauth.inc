<?php

function sm_fb_connect() {
    global $user;
    $fb= _sm_fb_init();
    $fb_id= $fb->getUser();
    if($fb_id && !$user->uid) {
        $info = $fb->api($fb_id);
        $account=sm_fb_get_local_user($fb_id);

        if(!$account) {
            $account= sm_fb_create_local_user($info);
        }
        if($account) {
            _sm_fb_external_login($fb_id,$account);
            if($_REQUEST['display']=='popup') {
                print "<script>opener.FBConnectReloadPage(); window.close();</script>";
            } else {
                drupal_goto('user');
            }
        }
    } else {
        $params=_sm_fb_params();
        $login_url=$fb->getLoginUrl($params);
         header("location:$login_url");
       // print "<a class='btn-blue' href='$login_url'>".t("Oops, something went wrong. Please try again")."</a>";
        
    }

}


function _sm_fb_get_uid($fbu) {
    $result = db_result(db_query("SELECT uid FROM {fb_user} WHERE fbu=%d", array(
    $fbu,
    )));
    return $result;
}

function sm_fb_create_local_user($info) {


    $fbu=$info['id'];
    $account = sm_fb_get_local_user($fbu);

    if(!$account && $fbu) {
        $account = sm_fb_get_local_user_by_email($info);
        _sm_fb_set_map($account, $fbu);
        sm_fb_invoke('profile_save', $info, $account, 'sm_fb');
    }

    if(!$account ) {
        if(isset($info['username'])) {
            $username=$info['username'];
        } else {
            $username = "$fbu@facebook";
        }
        if(!isset($info['email'])) {
            $info['email']= "$fbu@facebook";
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
        'init' => $info['email'],
        'status' => 1,
        );
        $defaults['mail'] = $info['email'];

        $edit = array_merge($defaults, $edit);
        if ($uid = db_result(db_query("SELECT uid FROM {users} WHERE name='%s'", $edit['name']))) {
            // The desired name is taken.
            watchdog('fb_user', 'Failed to create new user %name.  That name is already in the users table.',
            array('%name' => $edit['name']),
            WATCHDOG_ERROR, l(t('view user'), 'user/' . $uid));
        }
        else {
            $account = user_save('', $edit);
            _sm_fb_set_map($account, $fbu);
            sm_fb_invoke('profile_save', $info, $account, 'sm_fb');
            watchdog('fb_user', 'New user: %name %email.',
            array('%name' => $account->name, '%email' => '<'. $account->mail .'>'),
            WATCHDOG_NOTICE, l(t('edit'), 'user/'. $account->uid .'/edit'));


        }

    }

    return $account;

}

function sm_fb_get_local_user($fbu) {
    if ($uid = _sm_fb_get_uid($fbu)) {
        return user_load($uid);
    }
}

function  _sm_fb_set_map($account, $fbu) {

    if ($fbu &&
    $account->uid != 0) {
        // Delete any pre-existing mapping that might exist for this local uid or fbu.
        db_query("DELETE FROM {fb_user} WHERE uid=%d OR fbu=%d", $account->uid, $fbu);

        // Create the new mapping.
        db_query("INSERT INTO {fb_user} (uid, fbu) VALUES (%d, %d)", $account->uid, $fbu);


        watchdog('fb_user', 'Using fb_user to associate user !user with facebook user id %fbu.',
        array('!user' => l($account->name, 'user/' . $account->uid),
        '%fbu' => $fbu,
        ));

    }

}

function _sm_fb_external_login($fbu,$account) {
    if ($account &&
    $account->uid == $GLOBALS['user']->uid) {
        return $account;
    }
    elseif ($fbu &&
    $account &&
    $account->uid != $GLOBALS['user']->uid ) {
        $session_id = session_id();
        watchdog("fb_user", "fb_user_fb changing user to $account->uid");
        // user_external_login() fails if already logged in, so log out first.
        if ($GLOBALS['user']->uid) {
            session_destroy();
            $GLOBALS['user'] = drupal_anonymous_user();
        }

        $valid_user = user_external_login($account);

        // Special effort to support browsers without third-party cookies.
        /* if (function_exists('fb_sess_regenerate_hack')) {
        fb_sess_regenerate_hack();
        }*/
        if ($valid_user) {
            // Session changed after external login.  Invoking hook here allows modules to drupal_set_message().
            // fb_invoke(FB_USER_OP_POST_EXTERNAL_LOGIN, array('account' => $account), NULL, 'fb_user');
            return $account;
        }
    }
    return FALSE;
}

function sm_fb_get_local_user_by_email($info) {

    if (isset($info['id'])) {
        try {
            if (isset($info['email']) &&
            ($email =$info['email'])) {
                return user_load(array('mail' => $email));
            }
        }
        catch (Exception $e) {
            // This can occur when user logs out of facebook in another window, then returns to our site.

            // t('Failed to get google user email.');

        }
    }
}


