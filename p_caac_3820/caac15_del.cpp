/*************************************************
Copyright:   Baosight Software LTD.co Copyright (c) 2010
Author:   zhoabin
Version:    1.0
Date:     2020-04-09
Description: 会计期删除
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件
 

//外部函数声明

/*<remark>=========================================================
/// <summary>
/// 成本科目新增
/// <para>
/// 1.根据传入的前台数据，进tcaac15表删除数据。
/// </para>
/// <para>数据库表：TCAAC15(会计期信息表)         </para>
/// <para>主调用函数：前台FormCAAC15画面(F4)调用。   </para>
/// </summary>
/// <param name="TCAAC15">整表数据    </param>
/// <returns>处理结果</returns>
===========================================================</remark>*/

// service入口
BM2F_ENTERACE(caac15_del)

int f_caac15_del(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);

    /* 程序内部变量 */
	int   doFlag = 0;
    int   fetchRowCount = 0;
    int   v_count=0;

    /* 业务变量 */
	CString  datetime("");

	/* 实体类定义 */
	CModel tcaac15("TCAAC15");

	// 数据库SQL操作字符串
	CString  sql("");

    try
    { 
        //获得传入的数据行数
		int count = bcls_rec->Tables[0].Rows.get_Count();
		
		for (int i = 0; i <  count ; i++ )
		{
            //重置头文件,获得头文件中的默认值
			tcaac15.Reset();

			//取得一行数据， MergeFrom方法将获得bcls_rec指定表的指定行的数据，并赋值给CTCAAC02类型的对象
			tcaac15.MergeFrom(bcls_rec->Tables[0].Rows[i]);
            tcaac15.Delete("ACCOUNT_PERIOD");
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
