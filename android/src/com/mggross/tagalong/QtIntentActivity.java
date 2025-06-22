package com.mggross.tagalong;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import org.qtproject.qt.android.QtActivityBase;

public class QtIntentActivity extends QtActivityBase
{
    public static native void setDocument(String url);

    public void dispatchIntent()
    {
        handleIntent(getIntent());
    }

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        Intent intent = getIntent();
        if (intent != null)
        {
            setIntent(intent);
        }
    }

    @Override
    protected void onNewIntent(Intent intent)
    {
        super.onNewIntent(intent);
        setIntent(intent);
        handleIntent(intent);
    }

    private void handleIntent(Intent intent)
    {
        if (intent.getAction().equals("android.intent.action.VIEW"))
        {
            Uri data = intent.getData();
            String url = data.toString();
            setDocument(url);
        }
    }
}
