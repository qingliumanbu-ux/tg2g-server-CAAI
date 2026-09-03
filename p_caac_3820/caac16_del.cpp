/*************************************************
Copyright:   Baosight Software LTD.co Copyright (c) 2010
Author:   zhoabin
Version:    1.0
Date:     2020-07-20
Description: 加工费删除
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件
#include "tcaac16.h" 

//外部函数声明


// service入口
BM2F_ENTERACE(caac16_del)

int f_caac16_del(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);

    /* 程序内部变量 */
	int   doFlag = 0;
    int   fetchRowCount = 0;
    int   v_count=0;

    /* 业务变量 */
	CString  datetime("");

	/* 实体类定义 */
	CModel tcaac16("TCAAC16");

	// 数据库SQL操作字符串
	CString  sql("");

    try
    { 
        //获得传入的数据行数
		int count = bcls_rec->Tables[0].Rows.get_Count();
		
		for (int i = 0; i <  count ; i++ )
		{
            //重置头文件,获得头文件中的默认值
			tcaac16.Reset();
			tcaac16.MergeFrom(bcls_rec->Tables[0].Rows[i]);
            tcaac16.Delete("OUT_TASK_NO,PROC_MODE,OUT_UNIT_CODE,RECV_DEPT,THICK_MAX,THICK_MIN,EFFECT_DATE,PRICE_UNIT");
		}  
	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,"数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。" /* _RES("GCRSS0000006")*//*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sql + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		EDLog(1,1, "[%s]", s.sysmsg);
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg)-1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	return doFlag;
}
