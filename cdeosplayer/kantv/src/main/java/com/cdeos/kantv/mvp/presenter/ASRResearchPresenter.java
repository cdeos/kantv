package com.cdeos.kantv.mvp.presenter;

import android.content.Context;

import com.cdeos.kantv.utils.interf.presenter.BaseMvpPresenter;


public interface ASRResearchPresenter extends BaseMvpPresenter {
    void refreshVideo(Context context, boolean reScan);
}