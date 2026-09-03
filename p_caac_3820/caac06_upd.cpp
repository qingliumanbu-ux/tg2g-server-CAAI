/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhl
Version:    1.0
Date:     2016-03-10
Description: 会计期修改
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caac06_upd)

int f_caac06_upd(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);

    /* 程序内部变量 */
	int   doFlag = 0;
    int   fetchRowCount = 0;
	

    /* 业务变量 */
	CString  datetime("");

	/* 实体类定义 */
    CModel tcaac06("TCAAC06");

	// 数据库SQL操作字符串
	CString  sql("");


    try
    { 
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
        //获得传入的数据行数
		int count = bcls_rec->Tables[0].Rows.get_Count();
		
		for (int i = 0; i <  count ; i++ )
		{
            //重置头文件,获得头文件中的默认值
			tcaac06.Reset();

			//取得一行数据， MergeFrom方法将获得bcls_rec指定表的指定行的数据
			tcaac06.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			if (tcaac06["STATS_PERIOD"].ToString().Trim() == "")
			{
				sprintf(s.msg,"统计期为空!");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			if(tcaac06["S_DATETIME"].ToString().Trim() == "")
			{
				sprintf(s.msg,"开始时间为空!");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			if(tcaac06["E_DATETIME"].ToString().Trim() == "")
			{
				sprintf(s.msg,"结束时间为空!");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			//修改信息
			tcaac06["REC_REVISOR"]=s.userid;   //记录修改责任者
			tcaac06["REC_REVISE_TIME"]=datetime;   //记录修改时刻

            //修改一条记录
            tcaac06.Update("REC_REVISOR,REC_REVISE_TIME,STATS_PERIOD,S_DATETIME,E_DATETIME,CURRENT_STATUS,BACK_C1","STATS_PERIOD,DEPT_CODE");
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
