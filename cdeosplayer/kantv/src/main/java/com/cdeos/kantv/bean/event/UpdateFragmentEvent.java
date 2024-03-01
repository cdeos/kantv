package com.cdeos.kantv.bean.event;

import com.cdeos.kantv.ui.fragment.PersonalFragment;
import com.cdeos.kantv.ui.fragment.LocalMediaFragment;


public class UpdateFragmentEvent {
    private Class clazz;
    private int updateType;

    private UpdateFragmentEvent(Class clazz, int updateType){
        this.clazz = clazz;
        this.updateType = updateType;
    }

    public Class getClazz() {
        return clazz;
    }

    public int getUpdateType() {
        return updateType;
    }

    public static UpdateFragmentEvent updatePersonal() {
        return new UpdateFragmentEvent(PersonalFragment.class, -1);
    }

    public static UpdateFragmentEvent updatePlay(int updateType) {
        return new UpdateFragmentEvent(LocalMediaFragment.class, updateType);
    }
}